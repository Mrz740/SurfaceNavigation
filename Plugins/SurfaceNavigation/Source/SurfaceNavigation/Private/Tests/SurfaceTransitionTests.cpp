#include "Misc/AutomationTest.h"
#include "Utils/SurfaceTransitionTestUtils.h"

#if WITH_DEV_AUTOMATION_TESTS

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FSurfaceTransitionRequestAcceptance, "SurfaceNavigation.Movement.Transition.RequestAcceptance",
                                 EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)

bool FSurfaceTransitionRequestAcceptance::RunTest(const FString& Parameters)
{
	using namespace SurfaceTransitionTestUtils;
	const FSurfaceMovementTestWorld TestWorld = FSurfaceMovementTestWorld();

	FTransitionFixture Fixture = BuildQuadraticReorientationFixture(TestWorld, true);
	FSurfaceWaypoint Arrival = Fixture.Path.Waypoints[1];

	const FVector ActorLocation = Fixture.MovementActor->GetActorLocation();

	const bool bResult1 = TestTrue(TEXT("Movement component should reach the surface and start crawling before the transition is requested"), PrimeCrawlingAttachment(TestWorld,Fixture.MovementComponent));
	const bool bResult2 = TestTrue(TEXT("RequestTransition should accept a request when no transition is pending or active"),Fixture.MovementComponent->RequestTransition(Arrival.Position,Arrival.TransitionInfo.GetValue()));
	const bool bResult3 = TestEqual(TEXT("Accepting a request should publish Pending as the transition status"),Fixture.MovementComponent->GetTransitionStatus(), ESurfaceTransitionStatus::Pending);
	const bool bResult4 = TestEqual(TEXT("RequestTransition should not move the actor; Read has not run yet"), Fixture.MovementActor->GetActorLocation(), ActorLocation);

	return bResult1 && bResult2 && bResult3 && bResult4;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FSurfaceTransitionBusyRequest, "SurfaceNavigation.Movement.Transition.BusyRequest",
								 EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)

bool FSurfaceTransitionBusyRequest::RunTest(const FString& Parameters)
{
	using namespace SurfaceTransitionTestUtils;
	const FSurfaceMovementTestWorld TestWorld = FSurfaceMovementTestWorld();

	FTransitionFixture Fixture = BuildQuadraticReorientationFixture(TestWorld, true);
	FSurfaceWaypoint Arrival = Fixture.Path.Waypoints[1];

	const FVector ActorLocation = Fixture.MovementActor->GetActorLocation();

	const bool bResult1 = TestTrue(TEXT("Movement component should reach the surface and start crawling before the transition is requested"), PrimeCrawlingAttachment(TestWorld,Fixture.MovementComponent));
	const bool bResult2 = TestTrue(TEXT("The first RequestTransition call should be accepted since no transition is pending or active"),Fixture.MovementComponent->RequestTransition(Arrival.Position,Arrival.TransitionInfo.GetValue()));
	const bool bResult3 = TestEqual(TEXT("Accepting the first request should publish Pending as the transition status"),Fixture.MovementComponent->GetTransitionStatus(), ESurfaceTransitionStatus::Pending);
	const bool bResult4 = TestEqual(TEXT("Accepting the first request should not move the actor; Read has not run yet"), Fixture.MovementActor->GetActorLocation(), ActorLocation);
	const bool bResult5 = TestFalse(TEXT("A second RequestTransition call should be rejected while a request is already pending"),Fixture.MovementComponent->RequestTransition(FVector(0,0,0),Arrival.TransitionInfo.GetValue()));
	const bool bResult6 = TestEqual(TEXT("Rejecting the busy request should leave the transition status unchanged at Pending"),Fixture.MovementComponent->GetTransitionStatus(), ESurfaceTransitionStatus::Pending);
	const bool bResult7 = TestEqual(TEXT("Rejecting the busy request should not move the actor"), Fixture.MovementActor->GetActorLocation(), ActorLocation);

	return bResult1 && bResult2 && bResult3 && bResult4 && bResult5 && bResult6 && bResult7;
}

#endif
