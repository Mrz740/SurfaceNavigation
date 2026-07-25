// Copyright (c) Mrz740. Licensed under the MIT License.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Core/SurfacePhaseParticipant.h"
#include "SurfaceMovementComponent.generated.h"

USTRUCT()
struct FSurfaceProbeResult
{
	GENERATED_BODY()
	
	UPROPERTY()
	bool bIsOnSurface = false;
	UPROPERTY()
	FVector SurfaceNormal = FVector::UpVector;
	UPROPERTY()
	FVector ImpactPoint = FVector::ZeroVector;
};

USTRUCT()
struct FSurfaceState
{
	GENERATED_BODY()
	
	UPROPERTY()
	bool bIsOnSurface = false;
	UPROPERTY()
	FVector SurfaceNormal = FVector::UpVector;
	UPROPERTY()
	FVector ImpactPoint = FVector::ZeroVector;
};

UCLASS(ClassGroup=(Movement), meta=(BlueprintSpawnableComponent))
class SURFACENAVIGATION_API USurfaceMovementComponent : public UActorComponent, public ISurfacePhaseParticipant
{
	GENERATED_BODY()
	
	friend class FSurfaceMovementReadPhaseTests;
	
	UPROPERTY()
	FSurfaceProbeResult PendingProbeResult;
	UPROPERTY()
	FSurfaceState CommittedState;
	UPROPERTY(EditAnywhere)
	float ProbeDistance = 100.0f;
	
public:
	USurfaceMovementComponent();

	virtual void ExecuteReadPhase() override;
	virtual void ExecuteSimulatePhase() override;
	virtual void ExecuteCommitPhase() override;
	
	const FSurfaceState& GetCommittedState() const { return CommittedState; };
};