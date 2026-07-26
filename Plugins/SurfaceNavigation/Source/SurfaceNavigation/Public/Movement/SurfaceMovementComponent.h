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

	UPROPERTY()
	FSurfaceProbeResult PendingProbeResult;
	UPROPERTY()
	FSurfaceState CommittedState;
	UPROPERTY()
	bool bHasPendingTarget = false;
	UPROPERTY()
	FVector PendingTarget;
	UPROPERTY()
	FVector PendingMoveDelta;
	UPROPERTY()
	float PendingSpeed = 0.f;
	UPROPERTY()
	float CommittedSpeed = 0.f;

	UPROPERTY(EditAnywhere)
	bool bSweepMovement = true;
	UPROPERTY(EditAnywhere)
	float DefaultAcceptanceRadius = 25.f;
	UPROPERTY(EditAnywhere)
	float AccelerationRate = 200.f;
	UPROPERTY(EditAnywhere)
	float MaxSpeed = 400.f;
	UPROPERTY(EditAnywhere)
	float ProbeDistance = 100.0f;
	UPROPERTY(EditAnywhere)
	float RotationSlerpSpeed = 5.f;

public:
	USurfaceMovementComponent();

	virtual void ExecuteReadPhase() override;
	virtual void ExecuteSimulatePhase() override;
	virtual void ExecuteCommitPhase() override;

	const FSurfaceState& GetCommittedState() const { return CommittedState; };
	const FSurfaceProbeResult& GetPendingProbeResult() const { return PendingProbeResult; };
	const float& GetDefaultAcceptanceRadius() const { return DefaultAcceptanceRadius; };

	UFUNCTION(BlueprintCallable)
	void SetMovementTarget(const FVector WorldTargetPosition);
};
