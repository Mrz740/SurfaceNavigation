#include "Misc/AutomationTest.h"
#include "Movement/SurfaceMovementComponent.h"
#include "Utils/SurfaceMovementTestWorld.h"

#if WITH_DEV_AUTOMATION_TESTS

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FSurfaceMovementReadPhaseTests, "SurfaceNavigation.Movement.ReadPhase",
                                 EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)

bool FSurfaceMovementReadPhaseTests::RunTest(const FString& Parameters)
{
	const FSurfaceMovementTestWorld TestWorld;
	TestWorld.SpawnFlatPrimitive(FVector::ZeroVector, FVector::UpVector, FVector(500, 500, 10));

	const AActor* TestActor = TestWorld.SpawnMovementActor(FVector(0, 0, 50));

	FHitResult SanityHit;
	const bool bSanity = TestWorld.GetWorld()->LineTraceSingleByChannel(SanityHit, TestActor->GetActorLocation(),
	                                                                    TestActor->GetActorLocation() - FVector(
		                                                                    0, 0, 100), ECC_WorldStatic);
	const bool bResult1 = TestTrue(TEXT("Raw trace from the test actor should hit the floor fixture"), bSanity);

	USurfaceMovementComponent* MovementComponent = TestActor->FindComponentByClass<USurfaceMovementComponent>();
	MovementComponent->ExecuteReadPhase();

	const bool bResult2 = TestTrue(
		TEXT("Read should populate PendingProbeResult.bIsOnSurface"),
		MovementComponent->GetPendingProbeResult().bIsOnSurface);
	const bool bResult3 = TestEqual(
		TEXT("Read should find the floor's Up normal"), MovementComponent->GetPendingProbeResult().SurfaceNormal,
		FVector::UpVector);
	const bool bResult4 = TestEqual(
		TEXT("CommittedState should remain untouched by Read"), MovementComponent->GetCommittedState().SurfaceNormal,
		FVector::UpVector);
	const bool bResult5 = TestEqual(
		TEXT("Actor transform should remain untouched by Read"), TestActor->GetActorLocation(), FVector(0, 0, 50));

	MovementComponent->ExecuteSimulatePhase();

	const bool bResult6 = TestEqual(
		TEXT("Actor transform should remain untouched by Simulate"), TestActor->GetActorLocation(), FVector(0, 0, 50));
	const bool bResult7 = TestEqual(
		TEXT("CommittedState should remain untouched by Simulate"),
		MovementComponent->GetCommittedState().SurfaceNormal, FVector::UpVector);

	return bResult1 && bResult2 && bResult3 && bResult4 && bResult5 && bResult6 && bResult7;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FSurfaceMovementOrientationTest, "SurfaceNavigation.Movement.OrientationConvergence",
                                 EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)

bool FSurfaceMovementOrientationTest::RunTest(const FString& Parameters)
{
	constexpr int TickCap = 60;
	constexpr float AngleTolerance = 0.01f;
	constexpr float FixedDeltaTime = 1.f / 60.f;

	const FSurfaceMovementTestWorld TestWorld = FSurfaceMovementTestWorld();

	TestWorld.SpawnFlatPrimitive(FVector::ZeroVector, FVector::UpVector, FVector(500, 500, 10));
	const AActor* MoverA = TestWorld.SpawnMovementActor(FVector(0, 0, 50));
	USurfaceMovementComponent* CompA = MoverA->FindComponentByClass<USurfaceMovementComponent>();

	for (int32 i = 0; i < TickCap; ++i)
	{
		TestWorld.TickWorld(FixedDeltaTime);
		CompA->ExecuteReadPhase();
		CompA->ExecuteCommitPhase();
	}

	const bool bResult1 = TestTrue(
		TEXT("Floor fixture: CommittedState.bIsOnSurface should be true after repeated Read+Commit ticks"),
		CompA->GetCommittedState().bIsOnSurface);
	const bool bResult2 = TestGreaterThan(
		TEXT("Floor fixture: actor's Up vector should converge to the floor's normal (UpVector)"),
		static_cast<float>(FVector::DotProduct(MoverA->GetActorUpVector(), FVector::UpVector)), (1.f - AngleTolerance));

	const FVector RampNormal = FVector(0.6, 0, 0.8);

	TestWorld.SpawnFlatPrimitive(FVector(1000, 0, 0), RampNormal, FVector(500, 500, 10));
	const AActor* MoverB = TestWorld.SpawnMovementActor(FVector(1000, 0, 50));
	USurfaceMovementComponent* CompB = MoverB->FindComponentByClass<USurfaceMovementComponent>();

	for (int32 i = 0; i < TickCap; ++i)
	{
		TestWorld.TickWorld(FixedDeltaTime);
		CompB->ExecuteReadPhase();
		CompB->ExecuteCommitPhase();
	}
	const bool bResult3 = TestTrue(
		TEXT("Ramp fixture: CommittedState.bIsOnSurface should be true after repeated Read+Commit ticks"),
		CompB->GetCommittedState().bIsOnSurface);
	const bool bResult4 = TestGreaterThan(
		TEXT("Ramp fixture: actor's Up vector should converge to the ramp's tilted normal, not world-up"),
		static_cast<float>(FVector::DotProduct(MoverB->GetActorUpVector(), RampNormal)), (1.f - AngleTolerance));
	const bool bResult5 = TestNotEqual(
		TEXT("Ramp fixture: CommittedState.SurfaceNormal should differ from the floor-default UpVector"),
		CompB->GetCommittedState().SurfaceNormal, FVector::UpVector);

	return bResult1 && bResult2 && bResult3 && bResult4 && bResult5;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FSurfaceMovementMovementTest, "SurfaceNavigation.Movement.MovementConvergence",
                                 EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)

bool FSurfaceMovementMovementTest::RunTest(const FString& Parameters)
{
	constexpr int TickCap = 300;
	constexpr float AngleTolerance = 0.01f;
	constexpr float FixedDeltaTime = 1.f / 60.f;

	const FSurfaceMovementTestWorld TestWorld = FSurfaceMovementTestWorld();

	TestWorld.SpawnFlatPrimitive(FVector::ZeroVector, FVector::UpVector, FVector(500, 500, 10));
	const AActor* MoverA = TestWorld.SpawnMovementActor(FVector(0, 0, 50));
	USurfaceMovementComponent* CompA = MoverA->FindComponentByClass<USurfaceMovementComponent>();

	const FVector TargetA = FVector(200, 0, 50);
	CompA->SetMovementTarget(TargetA);

	for (int32 i = 0; i < TickCap; ++i)
	{
		TestWorld.TickWorld(FixedDeltaTime);
		CompA->ExecuteReadPhase();
		CompA->ExecuteSimulatePhase();
		CompA->ExecuteCommitPhase();
	}

	const bool bResult1 = TestLessThan(
		TEXT("Floor fixture: actor should arrive within DefaultAcceptanceRadius of TargetA"),
		static_cast<float>((MoverA->GetActorLocation() - TargetA).Size()), CompA->GetDefaultAcceptanceRadius());
	const bool bResult2 = TestGreaterThan(
		TEXT("Floor fixture: actor's Up vector should remain aligned to the floor's normal (UpVector) after moving"),
		static_cast<float>(FVector::DotProduct(MoverA->GetActorUpVector(), FVector::UpVector)), (1.f - AngleTolerance));

	const FVector RampNormal = FVector(0.6, 0, 0.8);

	TestWorld.SpawnFlatPrimitive(FVector(1000, 0, 0), RampNormal, FVector(500, 500, 10));
	const AActor* MoverB = TestWorld.SpawnMovementActor(FVector(1000, 0, 50));
	USurfaceMovementComponent* CompB = MoverB->FindComponentByClass<USurfaceMovementComponent>();

	const FVector TargetB = MoverB->GetActorLocation() + FVector(0.8, 0, -0.6) * 200.f;
	CompB->SetMovementTarget(TargetB);

	for (int32 i = 0; i < TickCap; ++i)
	{
		TestWorld.TickWorld(FixedDeltaTime);
		CompB->ExecuteReadPhase();
		CompB->ExecuteSimulatePhase();
		CompB->ExecuteCommitPhase();
	}

	const bool bResult3 = TestLessThan(
		TEXT("Ramp fixture: actor should arrive within DefaultAcceptanceRadius of TargetB"),
		static_cast<float>((MoverB->GetActorLocation() - TargetB).Size()), CompB->GetDefaultAcceptanceRadius());
	const bool bResult4 = TestGreaterThan(
		TEXT("Ramp fixture: actor's Up vector should converge to the ramp's tilted normal while moving"),
		static_cast<float>(FVector::DotProduct(MoverB->GetActorUpVector(), RampNormal)), (1.f - AngleTolerance));

	return bResult1 && bResult2 && bResult3 && bResult4;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FSurfaceMovementRecoveryTest, "SurfaceNavigation.Movement.FallingRecovery",
                                 EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)

bool FSurfaceMovementRecoveryTest::RunTest(const FString& Parameters)
{
	constexpr int TickCap = 300;
	constexpr float FixedDeltaTime = 1.f / 60.f;

	const FSurfaceMovementTestWorld TestWorld = FSurfaceMovementTestWorld();

	TestWorld.SpawnFlatPrimitive(FVector::ZeroVector, FVector::UpVector, FVector(100, 500, 10));
	AActor* MoverA = TestWorld.SpawnMovementActor(FVector(0, 0, 50));
	USurfaceMovementComponent* CompA = MoverA->FindComponentByClass<USurfaceMovementComponent>();

	const FVector TargetA = FVector(200, 0, 50);
	CompA->SetMovementTarget(TargetA);

	bool bResult1 = false;
	for (int32 i = 0; i < TickCap; ++i)
	{
		TestWorld.TickWorld(FixedDeltaTime);
		CompA->ExecuteReadPhase();
		CompA->ExecuteSimulatePhase();
		CompA->ExecuteCommitPhase();

		if (i == 0)
		{
			bResult1 = TestEqual(TEXT(""), CompA->GetMovementMode(), ESurfaceMovementMode::Crawling);
		}
		if (!CompA->GetCommittedState().bIsOnSurface) break;
	}

	const bool bResult2 = TestFalse(
		TEXT("Falling detection: bIsOnSurface should flip false after the actor walks off the bounded floor's edge"),
		CompA->GetCommittedState().bIsOnSurface);
	const bool bResult3 = TestEqual(TEXT(""), CompA->GetMovementMode(), ESurfaceMovementMode::Falling);

	const FVector LocationAtFall = MoverA->GetActorLocation();

	for (int32 i = 0; i < 3; ++i)
	{
		TestWorld.TickWorld(FixedDeltaTime);
		CompA->ExecuteReadPhase();
		CompA->ExecuteSimulatePhase();
		CompA->ExecuteCommitPhase();
	}

	const bool bResult4 = TestEqual(
		TEXT("Falling stability: actor location should not change across further ticks while still off-surface"),
		MoverA->GetActorLocation(), LocationAtFall);

	const FVector NewPosition = FVector(0, 0, 50);
	MoverA->SetActorLocation(NewPosition);

	for (int32 i = 0; i < 3; ++i)
	{
		TestWorld.TickWorld(FixedDeltaTime);
		CompA->ExecuteReadPhase();
		CompA->ExecuteSimulatePhase();
		CompA->ExecuteCommitPhase();
	}

	const bool bResult5 = TestTrue(
		TEXT("Recovery repin: bIsOnSurface should heal back to true once repositioned over supporting geometry"),
		CompA->GetCommittedState().bIsOnSurface);
	const bool bResult6 = TestEqual(TEXT(""), CompA->GetMovementMode(), ESurfaceMovementMode::Crawling);

	return bResult1 && bResult2 && bResult3 && bResult4 && bResult5 && bResult6;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FSurfaceMovementDoubleBufferTest, "SurfaceNavigation.Movement.DoubleBufferCorrectness",
                                 EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)

bool FSurfaceMovementDoubleBufferTest::RunTest(const FString& Parameters)
{
	const FSurfaceMovementTestWorld TestWorld = FSurfaceMovementTestWorld();
	constexpr float FixedDeltaTime = 1.f / 60.f;

	const FVector RampNormal = FVector(0.6, 0, 0.8);

	TestWorld.SpawnFlatPrimitive(FVector::ZeroVector, FVector::UpVector, FVector(500, 500, 10));
	TestWorld.SpawnFlatPrimitive(FVector(1000, 0, 0), RampNormal, FVector(500, 500, 10));

	AActor* Mover = TestWorld.SpawnMovementActor(FVector(0, 0, 50));
	USurfaceMovementComponent* Comp = Mover->FindComponentByClass<USurfaceMovementComponent>();

	TestWorld.TickWorld(FixedDeltaTime);
	Comp->ExecuteReadPhase();
	Comp->ExecuteCommitPhase();

	const bool bResult1 = TestEqual(TEXT(""), Comp->GetCommittedState().SurfaceNormal, FVector::UpVector);

	Mover->SetActorLocation(FVector(1000, 0, 50));

	TestWorld.TickWorld(FixedDeltaTime);
	Comp->ExecuteReadPhase();

	const bool bResult2 = TestEqual(TEXT(""), Comp->GetPendingProbeResult().SurfaceNormal, RampNormal);
	const bool bResult3 = TestEqual(TEXT(""), Comp->GetCommittedState().SurfaceNormal, FVector::UpVector);

	Comp->ExecuteCommitPhase();

	const bool bResult4 = TestEqual(TEXT(""), Comp->GetCommittedState().SurfaceNormal, RampNormal);

	return bResult1 && bResult2 && bResult3 && bResult4;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FSurfaceMovementSweepCollision, "SurfaceNavigation.Movement.SweepCollision",
								 EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)

bool FSurfaceMovementSweepCollision::RunTest(const FString& Parameters)
{
	const FSurfaceMovementTestWorld TestWorld = FSurfaceMovementTestWorld();

	TestWorld.SpawnFlatPrimitive(FVector(100,0,50), FVector::UpVector, FVector(20, 20, 20));

	AActor* Mover = TestWorld.SpawnMovementActor(FVector(0, 0, 50));

	FHitResult Hit;
	Mover->AddActorWorldOffset(FVector(200,0,0), true, &Hit);

	const bool bResult1 = TestTrue(TEXT("Sweep toward a blocking box should report a blocking hit"), Hit.bBlockingHit);

	return bResult1;
}

#endif // WITH_DEV_AUTOMATION_TESTS
