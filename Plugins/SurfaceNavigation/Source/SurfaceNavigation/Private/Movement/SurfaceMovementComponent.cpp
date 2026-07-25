// Copyright (c) Mrz740. Licensed under the MIT License.

#include "Movement/SurfaceMovementComponent.h"


// Sets default values for this component's properties
USurfaceMovementComponent::USurfaceMovementComponent()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = false;

	// ...
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
		Origin,Origin + Direction * ProbeDistance, ECC_WorldStatic, Params);
	
	PendingProbeResult.bIsOnSurface = bHit;
	PendingProbeResult.SurfaceNormal = bHit ? Hit.ImpactNormal : CommittedState.SurfaceNormal;
	PendingProbeResult.ImpactPoint = Hit.ImpactPoint;
}

void USurfaceMovementComponent::ExecuteSimulatePhase()
{
}

void USurfaceMovementComponent::ExecuteCommitPhase()
{
}

