#pragma once

#include "CoreMinimal.h"
#include "SurfaceMovementTestWorld.h"
#include "Movement/SurfaceMovementComponent.h"
#include "Pathfinding/SurfacePath.h"

#if WITH_DEV_AUTOMATION_TESTS

namespace SurfaceTransitionTestUtils
{
	struct FTransitionFixture
	{
		FSurfacePath Path;
		AActor* MovementActor = nullptr;
		USurfaceMovementComponent* MovementComponent = nullptr;
		AActor* ArrivalPrimitive = nullptr;
	};

	struct FSurfaceTransitionTestAccess
	{
		static const FActiveSurfaceTransition* GetActiveTransition(const USurfaceMovementComponent* Component)
		{
			return Component->ActiveTransition.IsSet() ? &Component->ActiveTransition.GetValue() : nullptr;
		}

		static bool HasPendingTarget(const USurfaceMovementComponent* Component)
		{
			return Component->bHasPendingTarget;
		}

		static FVector GetPendingTarget(const USurfaceMovementComponent* Component)
		{
			return Component->PendingTarget;
		}
	};

	FTransitionFixture BuildQuadraticReorientationFixture(const FSurfaceMovementTestWorld& World,
		bool bIncludeArrivalPrimitive = true);

	FTransitionFixture BuildStraightGapFixture(const FSurfaceMovementTestWorld& World,
		bool bIncludeArrivalPrimitive = true);

	AActor* AddBlockingBoxAcrossQuadraticCurve(const FSurfaceMovementTestWorld& World,
		float TransitionArcHeight);

	bool PrimeCrawlingAttachment(const FSurfaceMovementTestWorld& World, USurfaceMovementComponent* MovementComponent,
		int32 MaxIterations = 5, float DeltaTime = 1.f / 60.f);

	bool TickUntilTransitionStatus(const FSurfaceMovementTestWorld& World, USurfaceMovementComponent* MovementComponent,
		ESurfaceTransitionStatus TargetStatus, int32 MaxIterations = 5, float DeltaTime = 1.f / 60.f);

	bool TickUntilAwaitingArrivalRepin(const FSurfaceMovementTestWorld& World, USurfaceMovementComponent* MovementComponent,
		int32 MaxIterations = 200, float DeltaTime = 1.f / 60.f);
}

#endif