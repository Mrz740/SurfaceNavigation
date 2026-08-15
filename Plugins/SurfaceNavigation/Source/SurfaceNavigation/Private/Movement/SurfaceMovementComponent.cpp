// Copyright (c) Mrz740. Licensed under the MIT License.

#include "Movement/SurfaceMovementComponent.h"

bool USurfaceMovementComponent::HandleTransitionState()
{
	if (PendingTransitionRequest.IsSet())
	{
		if (MovementMode != ESurfaceMovementMode::Crawling)
		{
			return RejectPendingTransition();
		}

		if (PendingTransitionRequest->DestinationPosition.ContainsNaN() ||
			PendingTransitionRequest->TransitionInfo.DepartureNormal.ContainsNaN() ||
			PendingTransitionRequest->TransitionInfo.ArrivalNormal.ContainsNaN())
		{
			return RejectPendingTransition();
		}

		if (!(PendingTransitionRequest->TransitionInfo.bRequiresReorientation ||
			PendingTransitionRequest->TransitionInfo.bIsGapBridge))
		{
			return RejectPendingTransition();
		}

		const FTransform DepartureTransform = GetOwner()->GetTransform();
		const FVector DestinationPosition = PendingTransitionRequest->DestinationPosition;
		const FVector DepartureNormal = PendingTransitionRequest->TransitionInfo.DepartureNormal.GetSafeNormal();
		const FVector ArrivalNormal = PendingTransitionRequest->TransitionInfo.ArrivalNormal.GetSafeNormal();

		if (DepartureNormal.IsNearlyZero(KINDA_SMALL_NUMBER) || ArrivalNormal.IsNearlyZero(KINDA_SMALL_NUMBER))
		{
			return RejectPendingTransition();
		}

		ESurfaceTransitionCurveKind CurveKind;
		FVector ControlPoint = FVector::ZeroVector;

		if (PendingTransitionRequest->TransitionInfo.bRequiresReorientation)
		{
			CurveKind = ESurfaceTransitionCurveKind::QuadraticBezier;
			const FVector AveragedNormal = (DepartureNormal + ArrivalNormal).GetSafeNormal();
			if (AveragedNormal.IsNearlyZero(KINDA_SMALL_NUMBER))
			{
				return RejectPendingTransition();
			}

			ControlPoint = ((DepartureTransform.GetLocation() + DestinationPosition) / 2) +
				2 * TransitionArcHeight * AveragedNormal;
		}
		else
		{
			CurveKind = ESurfaceTransitionCurveKind::Linear;
		}

		const float EffectiveSpeed = FMath::Max(CommittedSpeed * TransitionSpeedMultiplier, MinimumTransitionSpeed);

		if (EffectiveSpeed <= 0)
		{
			return RejectPendingTransition();
		}

		TStaticArray<float, 17> Table = BuildCumulativeDistanceTable(CurveKind, DepartureTransform.GetLocation(),
			ControlPoint, DestinationPosition);
		const float TotalDistance = Table[16];

		if (!FMath::IsFinite(TotalDistance) || FMath::IsNearlyZero(TotalDistance))
		{
			return RejectPendingTransition();
		}

		ActiveTransition = FActiveSurfaceTransition{
			.DepartureTransform = DepartureTransform,
			.DestinationPosition = DestinationPosition,
			.DepartureNormal = DepartureNormal,
			.ArrivalNormal = ArrivalNormal,
			.CurveKind = CurveKind,
			.ControlPoint = ControlPoint,
			.EffectiveSpeed = EffectiveSpeed,
			.CumulativeDistanceTable = Table,
			.TotalDistance = TotalDistance
		};

		PendingTransitionRequest.Reset();
		bHasPendingTarget = false;
		PendingTransitionOutput = FSurfaceTransitionOutput{};
		MovementMode = ESurfaceMovementMode::Transitioning;
		TransitionStatus = ESurfaceTransitionStatus::Active;
		ArrivalRepinResult = EArrivalRepinResult::NotAttempted;

		return false;
	}

	if (MovementMode == ESurfaceMovementMode::Transitioning)
	{
		if (ActiveTransition->bAwaitingArrivalRepin)
		{
			const AActor* Owner = GetOwner();
			const FVector Origin = Owner->GetActorLocation();
			const FVector Direction = -ActiveTransition->ArrivalNormal;

			FCollisionQueryParams Params;
			Params.AddIgnoredActor(Owner);

			FHitResult Hit;
			const bool bHit = GetWorld()->LineTraceSingleByChannel(Hit,Origin,
				Origin + Direction * ProbeDistance, ECC_WorldStatic,Params);

			PendingProbeResult.bIsOnSurface = bHit;
			PendingProbeResult.SurfaceNormal = Hit.ImpactNormal;
			PendingProbeResult.ImpactPoint = Hit.ImpactPoint;
			ArrivalRepinResult = bHit ? EArrivalRepinResult::Succeeded : EArrivalRepinResult::Failed;
		}
		else
		{
			PendingProbeResult.bIsOnSurface = CommittedState.bIsOnSurface;
			PendingProbeResult.SurfaceNormal = CommittedState.SurfaceNormal;
			PendingProbeResult.ImpactPoint = CommittedState.ImpactPoint;
		}
		return false;
	}

	return true;
}

bool USurfaceMovementComponent::RejectPendingTransition()
{
	PendingTransitionRequest.Reset();
	bHasPendingTarget = false;
	TransitionStatus = ESurfaceTransitionStatus::Rejected;
	return true;
}

FVector USurfaceMovementComponent::EvaluateTransitionCurve(ESurfaceTransitionCurveKind CurveKind, const FVector& Start,
	const FVector& ControlPoint, const FVector& Destination, float T)
{
	switch (CurveKind)
	{
	case ESurfaceTransitionCurveKind::Linear:
		return FMath::Lerp(Start, Destination, T);
	case ESurfaceTransitionCurveKind::QuadraticBezier:
		return FMath::Square(1 - T) * Start + 2 * (1 - T) * T * ControlPoint + FMath::Square(T) * Destination;
	default:
		return FVector::ZeroVector;
	}
}

TStaticArray<float, 17> USurfaceMovementComponent::BuildCumulativeDistanceTable(ESurfaceTransitionCurveKind CurveKind,
	const FVector& Start, const FVector& ControlPoint, const FVector& Destination)
{
	TStaticArray<float, 17> Table;
	Table[0] = 0.f;

	FVector PreviousPoint = EvaluateTransitionCurve(CurveKind, Start, ControlPoint, Destination, 0.f);

	for (int8 i = 1; i <= 16; i++)
	{
		const float T = i / 16.f;
		FVector Point = EvaluateTransitionCurve(CurveKind, Start, ControlPoint, Destination, T);
		Table[i] = Table[i - 1] + FVector::Distance(PreviousPoint, Point);
		PreviousPoint = Point;
	}
	return Table;
}

float USurfaceMovementComponent::LookupDistanceTableProgress(const TStaticArray<float, 17>& Table, const float TotalDistance,
	const float RequestedDistance)
{
	const float ClampedRequestedDistance = FMath::Clamp(RequestedDistance, 0.f, TotalDistance);

	if (ClampedRequestedDistance == TotalDistance)
	{
		return 1.0f;
	}
	if (ClampedRequestedDistance == 0.f)
	{
		return 0.f;
	}

	for (int8 i = 0; i < 16; i++)
	{
		if (ClampedRequestedDistance <= Table[i + 1])
		{
			if (FMath::IsNearlyZero(Table[i+1] - Table[i]))
			{
				return i / 16.f;
			}
			const float Alpha = (ClampedRequestedDistance - Table[i]) / (Table[i + 1] - Table[i]);
			return (i + Alpha) / 16.f;
		}
	}
	return 1.0f;
}

USurfaceMovementComponent::USurfaceMovementComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

void USurfaceMovementComponent::ExecuteReadPhase()
{
	if (!HandleTransitionState())
	{
		return;
	}

	const AActor* Owner = GetOwner();
	const FVector Origin = Owner->GetActorLocation();
	const FVector Direction = -CommittedState.SurfaceNormal;

	FCollisionQueryParams Params;
	Params.AddIgnoredActor(Owner);

	FHitResult Hit;
	const bool bHit = GetWorld()->LineTraceSingleByChannel(Hit,Origin,
	 Origin + Direction * ProbeDistance, ECC_WorldStatic,Params);

	if (!bHit && CommittedState.bIsOnSurface)
	{
		FHitResult RecoveryHit;
		const bool bRecoveryHit = GetWorld()->LineTraceSingleByChannel(RecoveryHit, Origin,
			Origin - Owner->GetActorUpVector() * ProbeDistance, ECC_WorldStatic, Params);

		PendingProbeResult.bIsOnSurface = bRecoveryHit;
		PendingProbeResult.SurfaceNormal = bRecoveryHit ? RecoveryHit.ImpactNormal : CommittedState.SurfaceNormal;
		PendingProbeResult.ImpactPoint = bRecoveryHit ? RecoveryHit.ImpactPoint : CommittedState.ImpactPoint;
	}
	else
	{
		PendingProbeResult.bIsOnSurface = bHit;
		PendingProbeResult.SurfaceNormal = bHit ? Hit.ImpactNormal : CommittedState.SurfaceNormal;
		PendingProbeResult.ImpactPoint = Hit.ImpactPoint;
	}
}

void USurfaceMovementComponent::ExecuteSimulatePhase()
{
	if (MovementMode == ESurfaceMovementMode::Transitioning)
	{
		if (!ensureMsgf(ActiveTransition.IsSet(),
			TEXT("MovementMode is Transitioning but ActiveTransition is unset")))
		{
			return;
		}

		if (ActiveTransition->bAwaitingArrivalRepin)
		{
			return;
		}

		const float NextDistance = FMath::Min(ActiveTransition->DistanceTravelled +
			ActiveTransition->EffectiveSpeed * GetWorld()->GetDeltaSeconds(),ActiveTransition->TotalDistance);
		const float NextProgress = LookupDistanceTableProgress(ActiveTransition->CumulativeDistanceTable,
			ActiveTransition->TotalDistance, NextDistance);
		const FVector NextPosition = EvaluateTransitionCurve(ActiveTransition->CurveKind,
			ActiveTransition->DepartureTransform.GetLocation(),ActiveTransition->ControlPoint,
			ActiveTransition->DestinationPosition, NextProgress);
		const FVector InterpolatedUp = FMath::Lerp(ActiveTransition->DepartureNormal, ActiveTransition->ArrivalNormal,
			NextProgress).GetSafeNormal();
		const FQuat NextRotation = FQuat::FindBetweenNormals(ActiveTransition->DepartureNormal, InterpolatedUp) *
			ActiveTransition->DepartureTransform.GetRotation();

		PendingTransitionOutput.TargetPosition = NextPosition;
		PendingTransitionOutput.TargetRotation = NextRotation;
		PendingTransitionOutput.ProposedDistance = NextDistance;
		PendingTransitionOutput.ProposedProgress = NextProgress;
		PendingTransitionOutput.bReachedEndpoint = (NextDistance == ActiveTransition->TotalDistance);
		PendingTransitionOutput.bHasTransitionTransform = true;

		return;
	}

	if (MovementMode != ESurfaceMovementMode::Crawling || !bHasPendingTarget)
	{
		PendingMoveDelta = FVector::ZeroVector;
		PendingSpeed = 0.f;
		return;
	}

	const FVector ToTarget = PendingTarget - GetOwner()->GetActorLocation();
	const FVector PlaneProjected = ToTarget - FVector::DotProduct(ToTarget, CommittedState.SurfaceNormal) *
		CommittedState.SurfaceNormal;

	if (PlaneProjected.SizeSquared() < DefaultAcceptanceRadius * DefaultAcceptanceRadius)
	{
		PendingMoveDelta = FVector::ZeroVector;
		PendingSpeed = 0.f;
		return;
	}

	const float NewSpeed = FMath::Min(CommittedSpeed + AccelerationRate * GetWorld()->GetDeltaSeconds(), MaxSpeed);
	PendingMoveDelta = PlaneProjected.GetSafeNormal() * NewSpeed * GetWorld()->GetDeltaSeconds();
	PendingSpeed = NewSpeed;
}

void USurfaceMovementComponent::ExecuteCommitPhase()
{
	if (MovementMode == ESurfaceMovementMode::Transitioning)
	{
		if (!ensureMsgf(ActiveTransition.IsSet(),
			TEXT("MovementMode is Transitioning but ActiveTransition is unset")))
		{
			return;
		}

		if (ArrivalRepinResult == EArrivalRepinResult::Succeeded)
		{
			MovementMode = ESurfaceMovementMode::Crawling;
			TransitionStatus = ESurfaceTransitionStatus::Completed;
			ArrivalRepinResult = EArrivalRepinResult::NotAttempted;
			ActiveTransition.Reset();

			CommittedState.bIsOnSurface = PendingProbeResult.bIsOnSurface;
			CommittedState.ImpactPoint = PendingProbeResult.ImpactPoint;
			CommittedState.SurfaceNormal = PendingProbeResult.SurfaceNormal;
		}
		else if (ArrivalRepinResult == EArrivalRepinResult::Failed)
		{
			ActiveTransition.Reset();
			bHasPendingTarget = false;
			PendingMoveDelta = FVector::ZeroVector;

			PendingSpeed = 0.f;
			CommittedSpeed = 0.f;
			MovementMode = ESurfaceMovementMode::Falling;
			TransitionStatus = ESurfaceTransitionStatus::FailedArrivalRepin;

			ArrivalRepinResult = EArrivalRepinResult::NotAttempted;
		}
		else if (PendingTransitionOutput.bHasTransitionTransform)
		{
			FHitResult SweepHit;
			GetOwner()->SetActorLocationAndRotation(PendingTransitionOutput.TargetPosition,
				PendingTransitionOutput.TargetRotation,true, &SweepHit);

			if (SweepHit.bBlockingHit)
			{
				ActiveTransition.Reset();
				bHasPendingTarget = false;
				PendingMoveDelta = FVector::ZeroVector;

				PendingSpeed = 0.f;
				CommittedSpeed = 0.f;
				MovementMode = ESurfaceMovementMode::Falling;
				TransitionStatus = ESurfaceTransitionStatus::Blocked;
			}
			else
			{
				ActiveTransition->DistanceTravelled = PendingTransitionOutput.ProposedDistance;
				ActiveTransition->AcceptedProgress = PendingTransitionOutput.ProposedProgress;
				if (PendingTransitionOutput.bReachedEndpoint)
				{
					ActiveTransition->bAwaitingArrivalRepin = true;
				}
			}
			PendingTransitionOutput = FSurfaceTransitionOutput{};
		}
		return;
	}

	CommittedState.bIsOnSurface = PendingProbeResult.bIsOnSurface;
	CommittedState.SurfaceNormal = PendingProbeResult.SurfaceNormal;
	CommittedState.ImpactPoint = PendingProbeResult.ImpactPoint;

	MovementMode = CommittedState.bIsOnSurface ? ESurfaceMovementMode::Crawling : ESurfaceMovementMode::Falling;

	if (CommittedState.bIsOnSurface)
	{
		const FQuat TargetRotation = FRotationMatrix::MakeFromZX(CommittedState.SurfaceNormal,
		                                                         GetOwner()->GetActorForwardVector()).ToQuat();
		const FQuat NewRotation = FQuat::Slerp(GetOwner()->GetActorRotation().Quaternion(), TargetRotation,
		                                       RotationSlerpSpeed * GetWorld()->GetDeltaSeconds());
		GetOwner()->SetActorRotation(NewRotation);
		GetOwner()->AddActorWorldOffset(PendingMoveDelta, bSweepMovement);
		CommittedSpeed = PendingSpeed;
	}
}

bool USurfaceMovementComponent::RequestTransition(const FVector& DestinationPosition,
	const FSurfaceTransitionInfo& TransitionInfo)
{
	if (PendingTransitionRequest.IsSet() || ActiveTransition.IsSet())
	{
		return false;
	}

	PendingTransitionRequest = FPendingSurfaceTransitionRequest{
		.DestinationPosition = DestinationPosition,
		.TransitionInfo = TransitionInfo
	};
	TransitionStatus = ESurfaceTransitionStatus::Pending;
	return true;
}

void USurfaceMovementComponent::SetMovementTarget(const FVector WorldTargetPosition)
{
	bHasPendingTarget = true;
	PendingTarget = WorldTargetPosition;
}