#include "SurfaceTransitionTestUtils.h"

#if WITH_DEV_AUTOMATION_TESTS

SurfaceTransitionTestUtils::FTransitionFixture SurfaceTransitionTestUtils::BuildQuadraticReorientationFixture(
	const FSurfaceMovementTestWorld& World, const bool bIncludeArrivalPrimitive)
{
	FTransitionFixture Fixture;

	const FVector DeparturePosition = FVector(50,0,50);
	FSurfaceWaypoint DepartureWaypoint = FSurfaceWaypoint{
		.Position = DeparturePosition, .NodeIndex = 0
	};

	const FVector ArrivalPosition = FVector(250,0,50);
	FSurfaceWaypoint ArrivalWaypoint = FSurfaceWaypoint{
		.Position = ArrivalPosition, .NodeIndex = 1,
		.TransitionInfo = FSurfaceTransitionInfo{
			.DepartureNormal = FVector::UpVector,
			.ArrivalNormal = FVector::BackwardVector,
			.bRequiresReorientation = true,
			.bIsGapBridge = false
		}
	};

	Fixture.Path.Waypoints = {DepartureWaypoint, ArrivalWaypoint};
	Fixture.MovementActor = World.SpawnMovementActor(DeparturePosition);
	Fixture.MovementComponent = Fixture.MovementActor->FindComponentByClass<USurfaceMovementComponent>();

	World.SpawnFlatPrimitive(FVector::ZeroVector, FVector::UpVector,FVector(100, 200, 10));

	if (bIncludeArrivalPrimitive)
	{
		Fixture.ArrivalPrimitive = World.SpawnFlatPrimitive(FVector(300,0,50), FVector::BackwardVector,
			FVector(100, 200, 10));
	}

	return Fixture;
}

SurfaceTransitionTestUtils::FTransitionFixture SurfaceTransitionTestUtils::BuildStraightGapFixture(
	const FSurfaceMovementTestWorld& World, const bool bIncludeArrivalPrimitive)
{
	FTransitionFixture Fixture;

	const FVector DeparturePosition = FVector(50,0,50);
	FSurfaceWaypoint DepartureWaypoint = FSurfaceWaypoint{
		.Position = DeparturePosition, .NodeIndex = 0
	};

	const FVector ArrivalPosition = FVector(250,0,50);
	FSurfaceWaypoint ArrivalWaypoint = FSurfaceWaypoint{
		.Position = ArrivalPosition, .NodeIndex = 1,
		.TransitionInfo = FSurfaceTransitionInfo{
			.DepartureNormal = FVector::UpVector,
			.ArrivalNormal = FVector::UpVector,
			.bRequiresReorientation = false,
			.bIsGapBridge = true
		}
	};

	Fixture.Path.Waypoints = {DepartureWaypoint, ArrivalWaypoint};
	Fixture.MovementActor = World.SpawnMovementActor(DeparturePosition);
	Fixture.MovementComponent = Fixture.MovementActor->FindComponentByClass<USurfaceMovementComponent>();

	World.SpawnFlatPrimitive(FVector::ZeroVector, FVector::UpVector,FVector(100, 200, 10));

	if (bIncludeArrivalPrimitive)
	{
		Fixture.ArrivalPrimitive = World.SpawnFlatPrimitive(FVector(300,0,0), FVector::UpVector,
			FVector(100, 200, 10));
	}

	return Fixture;
}

AActor* SurfaceTransitionTestUtils::AddBlockingBoxAcrossQuadraticCurve(const FSurfaceMovementTestWorld& World,
	const float TransitionArcHeight)
{
	const FVector ChordMidpoint = (FVector(50,0,50) + FVector(250, 0, 50)) / 2.0f;
	const FVector AverageNormal = (FVector::UpVector + FVector::BackwardVector).GetSafeNormal();
	const FVector CurveMidpoint = ChordMidpoint + TransitionArcHeight * AverageNormal;

	return World.SpawnFlatPrimitive(CurveMidpoint, FVector::UpVector, FVector(8, 80, 40));
}

bool SurfaceTransitionTestUtils::PrimeCrawlingAttachment(const FSurfaceMovementTestWorld& World,
	USurfaceMovementComponent* MovementComponent, const int32 MaxIterations, const float DeltaTime)
{
	for (int32 i = 0; i < MaxIterations; i++)
	{
		World.TickWorld(DeltaTime);
		MovementComponent->ExecuteReadPhase();
		MovementComponent->ExecuteCommitPhase();

		if (MovementComponent->GetCommittedState().bIsOnSurface)
		{
			return true;
		}
	}
	return false;
}

bool SurfaceTransitionTestUtils::TickUntilTransitionStatus(const FSurfaceMovementTestWorld& World,
	USurfaceMovementComponent* MovementComponent, const ESurfaceTransitionStatus TargetStatus, const int32 MaxIterations,
	const float DeltaTime)
{
	for (int32 i = 0; i < MaxIterations; i++)
	{
		World.TickWorld(DeltaTime);
		MovementComponent->ExecuteReadPhase();
		MovementComponent->ExecuteSimulatePhase();
		MovementComponent->ExecuteCommitPhase();

		if (MovementComponent->GetTransitionStatus() == TargetStatus)
		{
			return true;
		}
	}
	return false;
}

bool SurfaceTransitionTestUtils::TickUntilAwaitingArrivalRepin(const FSurfaceMovementTestWorld& World,
	USurfaceMovementComponent* MovementComponent, const int32 MaxIterations, const float DeltaTime)
{
	for (int32 i = 0; i < MaxIterations; i++)
	{
		World.TickWorld(DeltaTime);
		MovementComponent->ExecuteReadPhase();
		MovementComponent->ExecuteSimulatePhase();
		MovementComponent->ExecuteCommitPhase();

		if (const FActiveSurfaceTransition* ActiveTransition = FSurfaceTransitionTestAccess::GetActiveTransition(MovementComponent); ActiveTransition != nullptr && ActiveTransition->bAwaitingArrivalRepin)
		{
			return true;
		}
	}
	return false;
}

#endif