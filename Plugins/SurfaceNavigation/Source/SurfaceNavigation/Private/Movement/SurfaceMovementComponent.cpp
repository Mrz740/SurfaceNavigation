// Copyright (c) Mrz740. Licensed under the MIT License.

#include "Movement/SurfaceMovementComponent.h"

USurfaceMovementComponent::USurfaceMovementComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

void USurfaceMovementComponent::ExecuteReadPhase()
{
	const AActor* Owner = GetOwner();
	const FVector Origin = Owner->GetActorLocation();
	const FVector Direction = -CommittedState.SurfaceNormal;

	FCollisionQueryParams Params;
	Params.AddIgnoredActor(Owner);

	FHitResult Hit;
	const bool bHit = GetWorld()->LineTraceSingleByChannel(Hit,
	                                                       Origin, Origin + Direction * ProbeDistance, ECC_WorldStatic,
	                                                       Params);

	if (!bHit && CommittedState.bIsOnSurface)
	{
		FHitResult RecoveryHit;
		const bool bRecoveryHit = GetWorld()->LineTraceSingleByChannel(RecoveryHit, Origin,
		                                                               Origin - Owner->GetActorUpVector() *
		                                                               ProbeDistance, ECC_WorldStatic, Params);

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
	if (!CommittedState.bIsOnSurface || !bHasPendingTarget)
	{
		PendingMoveDelta = FVector::ZeroVector;
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
	CommittedState.bIsOnSurface = PendingProbeResult.bIsOnSurface;
	CommittedState.SurfaceNormal = PendingProbeResult.SurfaceNormal;
	CommittedState.ImpactPoint = PendingProbeResult.ImpactPoint;

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

void USurfaceMovementComponent::SetMovementTarget(const FVector WorldTargetPosition)
{
	bHasPendingTarget = true;
	PendingTarget = WorldTargetPosition;
}
