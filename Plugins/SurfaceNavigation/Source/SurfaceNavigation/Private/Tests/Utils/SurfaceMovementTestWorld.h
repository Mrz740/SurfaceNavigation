#pragma once

#include "CoreMinimal.h"

#if WITH_DEV_AUTOMATION_TESTS

class FSurfaceMovementTestWorld
{
public:
	FSurfaceMovementTestWorld();
	~FSurfaceMovementTestWorld();

	AActor* SpawnFlatPrimitive(const FVector& Location, const FVector& Normal, const FVector& Extent) const;
	AActor* SpawnMovementActor(const FVector& Location) const;
	void TickWorld(float DeltaTime) const;

	UWorld* GetWorld() const { return World; }

private:
	UWorld* World = nullptr;
};

#endif
