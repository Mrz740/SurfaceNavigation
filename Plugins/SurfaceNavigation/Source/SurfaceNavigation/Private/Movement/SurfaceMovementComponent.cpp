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
}

void USurfaceMovementComponent::ExecuteSimulatePhase()
{
}

void USurfaceMovementComponent::ExecuteCommitPhase()
{
}

