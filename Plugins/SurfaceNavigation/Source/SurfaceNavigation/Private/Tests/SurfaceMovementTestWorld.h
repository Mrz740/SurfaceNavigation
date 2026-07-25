#pragma once

class FSurfaceMovementTestWorld
{
public:
	FSurfaceMovementTestWorld();
	~FSurfaceMovementTestWorld();
	
	AActor* SpawnFlatPrimitive(const FVector& Location, const FVector& Normal, const FVector& Extent) const;
	
	UWorld* GetWorld() const { return World; }
	
private:
	UWorld* World = nullptr;
};
