#include "SurfaceMovementTestWorld.h"
#include "Misc/AutomationTest.h"
#include "Movement/SurfaceMovementComponent.h"

#if WITH_DEV_AUTOMATION_TESTS

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FSurfaceMovementReadPhaseTests, "SurfaceNavigation.Movement.ReadPhase",
                                 EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)

bool FSurfaceMovementReadPhaseTests::RunTest(const FString& Parameters)
{
	FSurfaceMovementTestWorld TestWorld;
	AActor* Floor = TestWorld.SpawnFlatPrimitive(FVector::ZeroVector,
		FVector::UpVector,FVector(500,500,10));
	
	AActor* TestActor = TestWorld.GetWorld()->SpawnActor<AActor>();
	USceneComponent* TestActorRoot = NewObject<USceneComponent>(TestActor);
	TestActor->SetRootComponent(TestActorRoot);
	TestActorRoot->RegisterComponent();
	TestActor->SetActorLocation(FVector(0,0,50));
	
	USurfaceMovementComponent* MovementComponent = NewObject<USurfaceMovementComponent>(TestActor);
	MovementComponent->RegisterComponent();
	
	FHitResult SanityHit;
	bool bSanity = TestWorld.GetWorld()->LineTraceSingleByChannel(SanityHit, TestActor->GetActorLocation(),TestActor->GetActorLocation() - FVector(0,0,100), ECC_WorldStatic);
	bool bResult1 = TestTrue(TEXT("Raw trace from the test actor should hit the floor fixture"), bSanity);
	
	MovementComponent->ExecuteReadPhase();
	
	bool bResult2 = TestTrue(TEXT("Read should populate PendingProbeResult.bIsOnSurface"), MovementComponent->PendingProbeResult.bIsOnSurface);
	bool bResult3 = TestEqual(TEXT("Read should find the floor's Up normal"), MovementComponent->PendingProbeResult.SurfaceNormal, FVector::UpVector);
	bool bResult4 = TestEqual(TEXT("CommittedState should remain untouched by Read"), MovementComponent->GetCommittedState().SurfaceNormal,FVector::UpVector);
	bool bResult5 = TestEqual(TEXT("Actor transform should remain untouched by Read"), TestActor->GetActorLocation(),FVector(0,0,50));
	
	return bResult1 && bResult2 && bResult3 && bResult4 && bResult5;
}

#endif // WITH_DEV_AUTOMATION_TESTS