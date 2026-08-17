// Copyright (c) Mrz740. Licensed under the MIT License.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Core/SurfacePhaseParticipant.h"
#include "Pathfinding/SurfacePath.h"
#include "SurfaceMovementComponent.generated.h"

#if WITH_DEV_AUTOMATION_TESTS
namespace SurfaceTransitionTestUtils
{
	struct FSurfaceTransitionTestAccess;
}
#endif

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

struct FPendingSurfaceTransitionRequest
{
	FVector DestinationPosition = FVector::ZeroVector;
	FSurfaceTransitionInfo TransitionInfo;
};

enum class ESurfaceTransitionCurveKind : uint8
{
	Linear,
	QuadraticBezier,
};

struct FActiveSurfaceTransition
{
	FTransform DepartureTransform = FTransform::Identity;
	FVector DestinationPosition = FVector::ZeroVector;
	FVector DepartureNormal = FVector::UpVector;
	FVector ArrivalNormal = FVector::UpVector;
	ESurfaceTransitionCurveKind CurveKind = ESurfaceTransitionCurveKind::Linear;
	FVector ControlPoint = FVector::ZeroVector;
	float EffectiveSpeed = 0.0f;
	TStaticArray<float, 17> CumulativeDistanceTable;
	float TotalDistance = 0.0f;
	float DistanceTravelled = 0.0f;
	float AcceptedProgress = 0.0f;
	bool bAwaitingArrivalRepin = false;
};

struct FSurfaceTransitionOutput
{
	FVector TargetPosition = FVector::ZeroVector;
	FQuat TargetRotation = FQuat::Identity;
	float ProposedDistance = 0.0f;
	float ProposedProgress = 0.0f;
	bool bReachedEndpoint = false;
	bool bHasTransitionTransform = false;
};

UENUM()
enum class ESurfaceMovementMode : uint8
{
	Crawling,
	Falling,
	Transitioning
};

UENUM()
enum class ESurfaceTransitionStatus : uint8
{
	None,
	Pending,
	Active,
	Completed,
	Rejected,
	FailedArrivalRepin,
	Blocked
};

enum class EArrivalRepinResult : uint8
{
	NotAttempted,
	Succeeded,
	Failed
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
	UPROPERTY(EditAnywhere, meta=(ClampMin="0.0"))
	float DefaultAcceptanceRadius = 25.f;
	UPROPERTY(EditAnywhere, meta=(ClampMin="0.0"))
	float AccelerationRate = 200.f;
	UPROPERTY(EditAnywhere, meta=(ClampMin="0.0"))
	float MaxSpeed = 400.f;
	UPROPERTY(EditAnywhere, meta=(ClampMin="0.0"))
	float ProbeDistance = 100.0f;
	UPROPERTY(EditAnywhere, meta=(ClampMin="0.0"))
	float RotationSlerpSpeed = 5.f;
	UPROPERTY(EditAnywhere, meta=(ClampMin="0.01"))
	float TransitionArcHeight = 50.f;
	UPROPERTY(EditAnywhere, meta=(ClampMin="0.01"))
	float TransitionSpeedMultiplier = 1.f;
	UPROPERTY(EditAnywhere, meta=(ClampMin="0.01"))
	float MinimumTransitionSpeed = 100.f;

	ESurfaceMovementMode MovementMode = ESurfaceMovementMode::Falling;
	EArrivalRepinResult ArrivalRepinResult = EArrivalRepinResult::NotAttempted;

	ESurfaceTransitionStatus TransitionStatus = ESurfaceTransitionStatus::None;
	TOptional<FPendingSurfaceTransitionRequest> PendingTransitionRequest;
	FSurfaceTransitionOutput PendingTransitionOutput;
	TOptional<FActiveSurfaceTransition> ActiveTransition;

	#if WITH_DEV_AUTOMATION_TESTS
		friend struct SurfaceTransitionTestUtils::FSurfaceTransitionTestAccess;
	#endif

	bool HandleTransitionState();
	bool RejectPendingTransition();
	bool TraceForSurface(const FVector& Direction, FHitResult& OutHit) const;

	void AbandonActiveTransition(const ESurfaceTransitionStatus Reason);

	static FVector EvaluateTransitionCurve(const ESurfaceTransitionCurveKind CurveKind, const FVector& Start,
		const FVector& ControlPoint, const FVector& Destination, const float T);
	static TStaticArray<float,17> BuildCumulativeDistanceTable(const ESurfaceTransitionCurveKind CurveKind,
		const FVector& Start, const FVector& ControlPoint, const FVector& Destination);
	static float LookupDistanceTableProgress(const TStaticArray<float,17>& Table, const float TotalDistance,
		const float RequestedDistance);

public:
	USurfaceMovementComponent();

	virtual void ExecuteReadPhase() override;
	virtual void ExecuteSimulatePhase() override;
	virtual void ExecuteCommitPhase() override;

	bool RequestTransition(const FVector& DestinationPosition, const FSurfaceTransitionInfo& TransitionInfo);

	UFUNCTION(BlueprintCallable)
	void SetMovementTarget(const FVector WorldTargetPosition);

	const FSurfaceState& GetCommittedState() const { return CommittedState; }
	const FSurfaceProbeResult& GetPendingProbeResult() const { return PendingProbeResult; }
	const float& GetDefaultAcceptanceRadius() const { return DefaultAcceptanceRadius; }

	ESurfaceMovementMode GetMovementMode() const { return MovementMode; }
	ESurfaceTransitionStatus GetTransitionStatus() const { return TransitionStatus; }
};