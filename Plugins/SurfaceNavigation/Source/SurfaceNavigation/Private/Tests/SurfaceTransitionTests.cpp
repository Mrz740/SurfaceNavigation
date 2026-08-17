#include "Misc/AutomationTest.h"
#include "Utils/SurfaceTransitionTestUtils.h"

#if WITH_DEV_AUTOMATION_TESTS

using namespace SurfaceTransitionTestUtils;

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FSurfaceTransitionRequestAcceptance, "SurfaceNavigation.Movement.Transition.RequestAcceptance",
                                 EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)

bool FSurfaceTransitionRequestAcceptance::RunTest(const FString& Parameters)
{
	const FSurfaceMovementTestWorld TestWorld = FSurfaceMovementTestWorld();

	FTransitionFixture Fixture = BuildQuadraticReorientationFixture(TestWorld, true);
	FSurfaceWaypoint Arrival = Fixture.Path.Waypoints[1];

	const FVector ActorLocation = Fixture.MovementActor->GetActorLocation();

	FVector RequestedDestination = Arrival.Position;
	FSurfaceTransitionInfo RequestedInfo = Arrival.TransitionInfo.GetValue();

	const bool bResult1 = TestTrue(TEXT("Movement component should reach the surface and start crawling before the transition is requested"),
		PrimeCrawlingAttachment(TestWorld,Fixture.MovementComponent));

	const FRotator ActorRotation = Fixture.MovementActor->GetActorRotation();

	const bool bResult2 = TestTrue(TEXT("RequestTransition should accept a request when no transition is pending or active"),
		Fixture.MovementComponent->RequestTransition(RequestedDestination,RequestedInfo));
	const bool bResult3 = TestEqual(TEXT("Accepting a request should publish Pending as the transition status"),
		Fixture.MovementComponent->GetTransitionStatus(), ESurfaceTransitionStatus::Pending);
	const bool bResult4 = TestEqual(TEXT("RequestTransition should not move the actor; Read has not run yet"),
		Fixture.MovementActor->GetActorLocation(), ActorLocation);

	RequestedDestination = FVector(9999.f,9999.f,9999.f);
	RequestedInfo.ArrivalNormal = FVector(0.f,1.f,0.f);

	const bool bResult5 = TestEqual(TEXT("RequestTransition should not rotate the actor; Read has not run yet"),
				Fixture.MovementActor->GetActorRotation(), ActorRotation);

	Fixture.MovementComponent->ExecuteReadPhase();

	const bool bResult6 = TestEqual(TEXT("Read must not rotate the actor; it only freezes the maneuver"),
		Fixture.MovementActor->GetActorRotation(), ActorRotation);

	const bool bResult7 = TestEqual(TEXT("Read should accept the pending request and publish Active"),
		Fixture.MovementComponent->GetTransitionStatus(), ESurfaceTransitionStatus::Active);
	const bool bResult8 = TestEqual(TEXT("Read must not move the actor; it only freezes the maneuver"),
		ActorLocation, Fixture.MovementActor->GetActorLocation());

	const FActiveSurfaceTransition* Active = FSurfaceTransitionTestAccess::GetActiveTransition(Fixture.MovementComponent);

	const bool bResult9 = TestNotNull(TEXT("An accepted request should produce an active transition snapshot"), Active);

	if (!bResult9)
	{
		return false;
	}

	const bool bResult10 = TestEqual(TEXT("The frozen destination must match the value published before the caller mutated it"),
		Active->DestinationPosition, Arrival.Position);
	const bool bResult11 = TestEqual(TEXT("The frozen arrival normal must match the value published before the caller mutated it"),
		Active->ArrivalNormal, Arrival.TransitionInfo->ArrivalNormal.GetSafeNormal());

	return bResult1 && bResult2 && bResult3 && bResult4 && bResult5 && bResult6 && bResult7 && bResult8 && bResult9 &&
		bResult10 && bResult11;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FSurfaceTransitionBusyRequest, "SurfaceNavigation.Movement.Transition.BusyRequest",
								 EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)

bool FSurfaceTransitionBusyRequest::RunTest(const FString& Parameters)
{
	const FSurfaceMovementTestWorld TestWorld = FSurfaceMovementTestWorld();

	FTransitionFixture Fixture = BuildQuadraticReorientationFixture(TestWorld, true);
	FSurfaceWaypoint Arrival = Fixture.Path.Waypoints[1];

	const FVector ActorLocation = Fixture.MovementActor->GetActorLocation();

	const bool bResult1 = TestTrue(TEXT("Movement component should reach the surface and start crawling before the transition is requested"),
		PrimeCrawlingAttachment(TestWorld,Fixture.MovementComponent));
	const bool bResult2 = TestTrue(TEXT("The first RequestTransition call should be accepted since no transition is pending or active"),
		Fixture.MovementComponent->RequestTransition(Arrival.Position,Arrival.TransitionInfo.GetValue()));
	const bool bResult3 = TestEqual(TEXT("Accepting the first request should publish Pending as the transition status"),
		Fixture.MovementComponent->GetTransitionStatus(), ESurfaceTransitionStatus::Pending);
	const bool bResult4 = TestEqual(TEXT("Accepting the first request should not move the actor; Read has not run yet"),
		Fixture.MovementActor->GetActorLocation(), ActorLocation);
	const bool bResult5 = TestFalse(TEXT("A second RequestTransition call should be rejected while a request is already pending"),
		Fixture.MovementComponent->RequestTransition(FVector(0,0,0),Arrival.TransitionInfo.GetValue()));
	const bool bResult6 = TestEqual(TEXT("Rejecting the busy request should leave the transition status unchanged at Pending"),
		Fixture.MovementComponent->GetTransitionStatus(), ESurfaceTransitionStatus::Pending);
	const bool bResult7 = TestEqual(TEXT("Rejecting the busy request should not move the actor"),
		Fixture.MovementActor->GetActorLocation(), ActorLocation);

	return bResult1 && bResult2 && bResult3 && bResult4 && bResult5 && bResult6 && bResult7;
}


IMPLEMENT_SIMPLE_AUTOMATION_TEST(FSurfaceTransitionOpposingNormals, "SurfaceNavigation.Movement.Transition.OpposingNormals",
								 EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)

bool FSurfaceTransitionOpposingNormals::RunTest(const FString& Parameters)
{
	const FSurfaceMovementTestWorld TestWorld = FSurfaceMovementTestWorld();

	FTransitionFixture Fixture = BuildQuadraticReorientationFixture(TestWorld, true);

	USurfaceMovementComponent* Comp = Fixture.MovementComponent;
	const AActor* Mover = Fixture.MovementActor;

	FSurfaceTransitionInfo OpposingInfo;
	OpposingInfo.DepartureNormal = FVector(0,0,1);
	OpposingInfo.ArrivalNormal = FVector(0,0,-1);
	OpposingInfo.bRequiresReorientation = true;

	const FVector Destination = Fixture.Path.Waypoints[1].Position;
	const FVector ActorLocation = Mover->GetActorLocation();

	const bool bResult1 = TestTrue(TEXT("Movement component should reach the surface and start crawling before the transition is requested"),
		PrimeCrawlingAttachment(TestWorld, Comp));

	const FRotator ActorRotation = Mover->GetActorRotation();

	Comp->SetMovementTarget(FVector(0,0,0));

	const bool bResult2 = TestTrue(TEXT("RequestTransition should accept opposing-normal metadata; admission never inspects normals"),
		Comp->RequestTransition(Destination, OpposingInfo));
	const bool bResult3 = TestEqual(TEXT("Accepting a request should publish Pending as the transition status"),
		Comp->GetTransitionStatus(), ESurfaceTransitionStatus::Pending);

	Comp->ExecuteReadPhase();

	const bool bResult4 = TestEqual(TEXT("Read should reject a maneuver whose averaged normal is too small to normalize"),
		Comp->GetTransitionStatus(), ESurfaceTransitionStatus::Rejected);
	const bool bResult5 = TestEqual(TEXT("Rejection should preserve the prior movement mode instead of entering Transitioning"),
		Comp->GetMovementMode(), ESurfaceMovementMode::Crawling);
	const bool bResult6 = TestEqual(TEXT("Read must not move the actor on a rejected request"),
		Mover->GetActorLocation(), ActorLocation);
	const bool bResult7 = TestEqual(TEXT("Read must not rotate the actor on a rejected request"),
		Mover->GetActorRotation(), ActorRotation);

	Comp->ExecuteSimulatePhase();
	Comp->ExecuteCommitPhase();

	const bool bResult8 = TestEqual(TEXT("A rejected request must clear the ordinary target so recovery cannot resume it"),
				Mover->GetActorLocation(), ActorLocation);
	const bool bResult9 = TestFalse(TEXT("Rejection must clear the ordinary target outright, not merely leave it unpursued"),
		FSurfaceTransitionTestAccess::HasPendingTarget(Comp));

	return bResult1 && bResult2 && bResult3 && bResult4 && bResult5 && bResult6 && bResult7 && bResult8 && bResult9;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FSurfaceTransitionCurveConstruction, "SurfaceNavigation.Movement.Transition.CurveConstruction",
								 EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)

bool FSurfaceTransitionCurveConstruction::RunTest(const FString& Parameters)
{
	const FSurfaceMovementTestWorld QuadTestWorld = FSurfaceMovementTestWorld();

	constexpr float ExpectedArcHeight = 50.f;
	constexpr float Tolerance = 1.f;

	const FTransitionFixture QuadFixture = BuildQuadraticReorientationFixture(QuadTestWorld, true);
	FSurfaceWaypoint QuadArrival = QuadFixture.Path.Waypoints[1];

	const bool bResult1 = TestTrue(TEXT("Movement component should reach the surface and start crawling before the transition is requested"),
		PrimeCrawlingAttachment(QuadTestWorld, QuadFixture.MovementComponent));
	const bool bResult2 = TestTrue(TEXT("RequestTransition should accept a request when no transition is pending or active"),
		QuadFixture.MovementComponent->RequestTransition(QuadArrival.Position, QuadArrival.TransitionInfo.GetValue()));

	QuadFixture.MovementComponent->ExecuteReadPhase();

	const FActiveSurfaceTransition* QuadActive = FSurfaceTransitionTestAccess::GetActiveTransition(QuadFixture.MovementComponent);
	const bool bResult3 = TestNotNull(TEXT("An accepted request should produce an active transition snapshot"), QuadActive);

	if (!bResult3)
	{
		return false;
	}

	const bool bResult4 = TestEqual(TEXT("Reorientation metadata should select the quadratic Bezier curve"),
		QuadActive->CurveKind, ESurfaceTransitionCurveKind::QuadraticBezier);

	const FVector Start = QuadActive->DepartureTransform.GetLocation();
	const FVector CurveMidpoint = 0.25f * Start + 0.5f * QuadActive->ControlPoint + 0.25 * QuadActive->DestinationPosition;
	const FVector ChordMidpoint = (Start + QuadActive->DestinationPosition) / 2.f;
	const float ClearanceOffset = FVector::Dist(CurveMidpoint, ChordMidpoint);

	const bool bResult5 = TestEqual(TEXT("The curve should reach its configured clearance at t=0.5"),
		ClearanceOffset, ExpectedArcHeight, Tolerance);
	const bool bResult6 = TestEqual(TEXT("The cumulative distance table should start at zero"),
		QuadActive->CumulativeDistanceTable[0],0.f);
	const bool bResult7 = TestEqual(TEXT("The cumulative distance table's final entry should equal the total curve length"),
		QuadActive->CumulativeDistanceTable[16], QuadActive->TotalDistance);

	bool bMonotonic = true;
	for (int32 i = 1; i <= 16; i++)
	{
		if (QuadActive->CumulativeDistanceTable[i] < QuadActive->CumulativeDistanceTable[i-1])
		{
			bMonotonic = false;
			break;
		}
	}
	const bool bResult8 = TestTrue(TEXT("The cumulative distance table should never decrease"), bMonotonic);

	const FSurfaceMovementTestWorld GapTestWorld = FSurfaceMovementTestWorld();

	FTransitionFixture GapFixture = BuildStraightGapFixture(GapTestWorld, true);
	FSurfaceWaypoint GapArrival = GapFixture.Path.Waypoints[1];

	const bool bResult9 = TestTrue(TEXT("Movement component should reach the surface and start crawling before the transition is requested"),
		PrimeCrawlingAttachment(GapTestWorld, GapFixture.MovementComponent));
	const bool bResult10 = TestTrue(TEXT("RequestTransition should accept a request when no transition is pending or active"),
		GapFixture.MovementComponent->RequestTransition(GapArrival.Position, GapArrival.TransitionInfo.GetValue()));

	GapFixture.MovementComponent->ExecuteReadPhase();

	const FActiveSurfaceTransition* GapActive = FSurfaceTransitionTestAccess::GetActiveTransition(GapFixture.MovementComponent);
	const bool bResult11 = TestNotNull(TEXT("An accepted request should produce an active transition snapshot"), GapActive);
	if (!bResult11)
	{
		return false;
	}

	const bool bResult12 = TestEqual(TEXT("A same-normal gap bridge should select the linear curve even though bIsGapBridge is true"),
		GapActive->CurveKind, ESurfaceTransitionCurveKind::Linear);
	const bool bResult13 = TestEqual(TEXT("A linear transition should never compute a control point or apply arc height"),
		GapActive->ControlPoint, FVector::ZeroVector);

	return bResult1 && bResult2 && bResult3 && bResult4 && bResult5 && bResult6 && bResult7 && bResult8 && bResult9 &&
		bResult10 && bResult11 && bResult12 && bResult13;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FSurfaceTransitionBezierTraversal, "SurfaceNavigation.Movement.Transition.BezierTraversal",
								 EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)

bool FSurfaceTransitionBezierTraversal::RunTest(const FString& Parameters)
{
	const FSurfaceMovementTestWorld TestWorld = FSurfaceMovementTestWorld();

	const FTransitionFixture Fixture = BuildQuadraticReorientationFixture(TestWorld, true);

	USurfaceMovementComponent* Comp = Fixture.MovementComponent;
	const AActor* Mover = Fixture.MovementActor;

	FSurfaceWaypoint Arrival = Fixture.Path.Waypoints[1];

	const bool bResult1 = TestTrue(TEXT("Movement component should reach the surface and start crawling before the transition is requested"),
		PrimeCrawlingAttachment(TestWorld, Comp));
	const bool bResult2 = TestTrue(TEXT("RequestTransition should accept a request when no transition is pending or active"),
		Comp->RequestTransition(Arrival.Position, Arrival.TransitionInfo.GetValue()));

	TestWorld.TickWorld(1/60.f);
	Comp->ExecuteReadPhase();

	const bool bResult3 = TestEqual(TEXT("Read should accept the pending request and publish Active"),
		Comp->GetTransitionStatus(), ESurfaceTransitionStatus::Active);

	const FVector StartLocation = Mover->GetActorLocation();
	const FRotator StartRotation = Mover->GetActorRotation();

	Comp->ExecuteSimulatePhase();

	const bool bResult4 = TestEqual(TEXT("Simulate must not move the actor; it only proposes the next transition transform"),
		Mover->GetActorLocation(), StartLocation);
	const bool bResult5 = TestEqual(TEXT("Simulate must not rotate the actor; it only proposes the next transition transform"),
		Mover->GetActorRotation(), StartRotation);

	Comp->ExecuteCommitPhase();

	const bool bResult6 = TestNotEqual(TEXT("Commit should apply the proposed transition position"),
		Mover->GetActorLocation(), StartLocation);
	const bool bResult7 = TestNotEqual(TEXT("Commit should apply the proposed transition rotation"),
		Mover->GetActorRotation(), StartRotation);

	const FActiveSurfaceTransition* ActiveTransition = FSurfaceTransitionTestAccess::GetActiveTransition(Comp);

	const bool bResult8 = TestNotNull(TEXT("An accepted request should produce an active transition snapshot"), ActiveTransition);

	if (!bResult8)
	{
		return false;
	}

	const double Step = ActiveTransition->EffectiveSpeed * (1.0 / 60.0);

	bool bResult9 = true;

	for (int32 i = 0; i < 4; i++)
	{
		const FVector PreviousLocation = Mover->GetActorLocation();
		TestWorld.TickWorld(1.0 / 60.f);
		Comp->ExecuteReadPhase();
		Comp->ExecuteSimulatePhase();
		Comp->ExecuteCommitPhase();

		bResult9 = TestEqual(TEXT("Every non-final committed step should travel the frozen effective speed times delta time"),
			FVector::Distance(Mover->GetActorLocation(), PreviousLocation), Step, 0.1) && bResult9;
	}

	const bool bResult10 = TestTrue(TEXT("The maneuver should reach its endpoint and await the arrival repin within the iteration cap"),
		TickUntilAwaitingArrivalRepin(TestWorld, Comp));

	ActiveTransition = FSurfaceTransitionTestAccess::GetActiveTransition(Comp);

	const bool bResult11 = TestNotNull(TEXT("The active transition snapshot must survive the endpoint Commit for the arrival repin"), ActiveTransition);

	if (!bResult11)
	{
		return false;
	}

	const bool bResult12 = TestEqual(TEXT("The endpoint Commit should place the actor at the frozen destination"),
		Mover->GetActorLocation(), Arrival.Position, KINDA_SMALL_NUMBER);
	const bool bResult13 = TestEqual(TEXT("Position and Up should reach their frozen arrival values together"),
		Mover->GetActorUpVector(), ActiveTransition->ArrivalNormal, KINDA_SMALL_NUMBER);

	const bool bResult14 = TestEqual(TEXT("Reaching the endpoint transform must not leave Transitioning; the repin has not run"),
		Comp->GetMovementMode(), ESurfaceMovementMode::Transitioning);
	const bool bResult15 = TestEqual(TEXT("Reaching the endpoint transform is not completion; status must remain Active"),
		Comp->GetTransitionStatus(),ESurfaceTransitionStatus::Active);

	const FVector EndLocation = Mover->GetActorLocation();
	const FRotator EndRotation = Mover->GetActorRotation();

	TestWorld.TickWorld(1/60.f);
	Comp->ExecuteReadPhase();
	Comp->ExecuteSimulatePhase();

	const bool bResult16 = TestEqual(TEXT("An endpoint-awaiting maneuver must produce no further movement while it waits for the repin"),
		Mover->GetActorLocation(), EndLocation);
	const bool bResult17 = TestEqual(TEXT("An endpoint-awaiting maneuver must produce no further rotation while it waits for the repin"),
		Mover->GetActorRotation(), EndRotation);
	const bool bResult18 = TestEqual(TEXT("Waiting frames must not publish a terminal status; completion belongs to the repin Commit"),
		Comp->GetTransitionStatus(), ESurfaceTransitionStatus::Active);

	return bResult1 && bResult2 && bResult3 && bResult4 && bResult5 && bResult6 && bResult7 && bResult8 && bResult9 &&
		bResult10 && bResult11 && bResult12 && bResult13 && bResult14 && bResult15 && bResult16 && bResult17 && bResult18;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FSurfaceTransitionStraightGapTraversal, "SurfaceNavigation.Movement.Transition.StraightGapTraversal",
								 EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)

bool FSurfaceTransitionStraightGapTraversal::RunTest(const FString& Parameters)
{
	const FSurfaceMovementTestWorld TestWorld = FSurfaceMovementTestWorld();

	const FTransitionFixture Fixture = BuildStraightGapFixture(TestWorld, true);

	USurfaceMovementComponent* Comp = Fixture.MovementComponent;
	const AActor* Mover = Fixture.MovementActor;

	const FSurfaceWaypoint Arrival = Fixture.Path.Waypoints[1];

	const bool bResult1 = TestTrue(TEXT("Movement component should reach the surface and start crawling before the transition is requested"),
			PrimeCrawlingAttachment(TestWorld, Comp));
	const bool bResult2 = TestTrue(TEXT("RequestTransition should accept a request when no transition is pending or active"),
		Comp->RequestTransition(Arrival.Position, Arrival.TransitionInfo.GetValue()));

	TestWorld.TickWorld(1/60.f);

	Comp->ExecuteReadPhase();

	const bool bResult3 = TestEqual(TEXT("Read should accept the pending request and publish Active"),
		Comp->GetTransitionStatus(), ESurfaceTransitionStatus::Active);

	const FActiveSurfaceTransition* ActiveTransition = FSurfaceTransitionTestAccess::GetActiveTransition(Comp);

	const bool bResult4 = TestNotNull(TEXT("An accepted request should produce an active transition snapshot"),
		ActiveTransition);

	if (!bResult4)
	{
		return false;
	}

	const FRotator StartRotation = Mover->GetActorRotation();

	const double Step = ActiveTransition->EffectiveSpeed * (1.0 / 60.0);

	bool bResult5 = true;
	for (int32 i = 0; i < 4; i++)
	{
		const FVector PreviousLocation = Mover->GetActorLocation();
		TestWorld.TickWorld(1/60.f);
		Comp->ExecuteReadPhase();
		Comp->ExecuteSimulatePhase();
		Comp->ExecuteCommitPhase();
		bResult5 = TestEqual(TEXT("A straight gap bridge should advance at the frozen effective speed on every non-final step"),
			FVector::Distance(Mover->GetActorLocation(), PreviousLocation), Step, 0.1) && bResult5;
	}

	const bool bResult6 = TestTrue(TEXT("The straight maneuver should reach its endpoint and await the arrival repin within the iteration cap"),
		TickUntilAwaitingArrivalRepin(TestWorld, Comp));

	ActiveTransition = FSurfaceTransitionTestAccess::GetActiveTransition(Comp);

	const bool bResult7 = TestNotNull(TEXT("The active transition snapshot must survive the endpoint Commit for the arrival repin"), ActiveTransition);

	if (!bResult7)
	{
		return false;
	}

	const bool bResult8 = TestEqual(TEXT("A gap bridge without reorientation should still reach the frozen destination"),
		Mover->GetActorLocation(), Arrival.Position, KINDA_SMALL_NUMBER);
	const bool bResult9 = TestEqual(TEXT("Up should end at the frozen arrival normal, catching an arc that rotated and rotated back"),
		Mover->GetActorUpVector(), ActiveTransition->ArrivalNormal, KINDA_SMALL_NUMBER);
	const bool bResult10 = TestEqual(TEXT("A same-normal transition must never rotate the actor; the interpolated Up delta is identity"),
		Mover->GetActorRotation(), StartRotation, KINDA_SMALL_NUMBER);

	return bResult1 && bResult2 && bResult3 && bResult4 && bResult5 && bResult6 && bResult7 && bResult8 && bResult9 && bResult10;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FSurfaceTransitionBlockedSweep, "SurfaceNavigation.Movement.Transition.BlockedSweep",
								 EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)

bool FSurfaceTransitionBlockedSweep::RunTest(const FString& Parameters)
{
	const FSurfaceMovementTestWorld TestWorld = FSurfaceMovementTestWorld();

	const FTransitionFixture Fixture = BuildQuadraticReorientationFixture(TestWorld, true);

	AddBlockingBoxAcrossQuadraticCurve(TestWorld, 50.f);

	USurfaceMovementComponent* Comp = Fixture.MovementComponent;
	const AActor* Mover = Fixture.MovementActor;

	const FSurfaceWaypoint Arrival = Fixture.Path.Waypoints[1];

	const bool bResult1 = TestTrue(TEXT("Movement component should reach the surface and start crawling before the transition is requested"),
			PrimeCrawlingAttachment(TestWorld, Comp));
	const bool bResult2 = TestTrue(TEXT("RequestTransition should accept a request when no transition is pending or active"),
		Comp->RequestTransition(Arrival.Position, Arrival.TransitionInfo.GetValue()));

	TestWorld.TickWorld(1/60.f);

	Comp->ExecuteReadPhase();

	const bool bResult3 = TestEqual(TEXT("Read should accept the pending request and publish Active"),
		Comp->GetTransitionStatus(), ESurfaceTransitionStatus::Active);

	Comp->SetMovementTarget(FVector(1000,1000,0));

	const bool bResult4 = TestTrue(TEXT("An obstacle across the arc should block the swept traversal within the iteration cap"),
		TickUntilTransitionStatus(TestWorld, Comp, ESurfaceTransitionStatus::Blocked, 200));

	const bool bResult5 = TestEqual(TEXT("A blocking sweep should publish Blocked as the transition status"),
		Comp->GetTransitionStatus(), ESurfaceTransitionStatus::Blocked);
	const bool bResult6 = TestEqual(TEXT("A blocking sweep should enter Falling; the actor is attached to neither surface"),
		Comp->GetMovementMode(), ESurfaceMovementMode::Falling);
	const bool bResult7 = TestNull(TEXT("A blocking sweep should clear the active transition so the maneuver cannot resume"),
		FSurfaceTransitionTestAccess::GetActiveTransition(Comp));

	const FVector BlockedLocation = Mover->GetActorLocation();

	for (int32 i = 0; i < 4; i++)
	{
		TestWorld.TickWorld(1/60.f);
		Comp->ExecuteReadPhase();
		Comp->ExecuteSimulatePhase();
		Comp->ExecuteCommitPhase();
	}

	const bool bResult8 = TestEqual(TEXT("Later frames must make no further curve progress and must not pursue the successor target"),
		Mover->GetActorLocation(), BlockedLocation);
	const bool bResult9 = TestEqual(TEXT("Blocked must persist as the pollable outcome until another request is admitted"),
		Comp->GetTransitionStatus(), ESurfaceTransitionStatus::Blocked);
	const bool bResult10 = TestFalse(TEXT("A blocking sweep must clear the successor target; Falling alone is not what keeps it unpursued"),
		FSurfaceTransitionTestAccess::HasPendingTarget(Comp));

	return bResult1 && bResult2 && bResult3 && bResult4 && bResult5 && bResult6 && bResult7 && bResult8 && bResult9 && bResult10;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FSurfaceTransitionEndpointRepinTiming, "SurfaceNavigation.Movement.Transition.EndpointRepinTiming",
								 EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)
bool FSurfaceTransitionEndpointRepinTiming::RunTest(const FString& Parameters)
{
	const FSurfaceMovementTestWorld TestWorld = FSurfaceMovementTestWorld();
	const FTransitionFixture Fixture = BuildQuadraticReorientationFixture(TestWorld, true);

	USurfaceMovementComponent* Comp = Fixture.MovementComponent;
	const AActor* Mover = Fixture.MovementActor;

	const FSurfaceWaypoint Arrival = Fixture.Path.Waypoints[1];

	const bool bResult1 = TestTrue(TEXT("Movement component should reach the surface and start crawling before the transition is requested"),
			PrimeCrawlingAttachment(TestWorld, Comp));

	const FVector DepartureNormal = Comp->GetCommittedState().SurfaceNormal;

	const bool bResult2 = TestTrue(TEXT("RequestTransition should accept a request when no transition is pending or active"),
		Comp->RequestTransition(Arrival.Position, Arrival.TransitionInfo.GetValue()));
	const bool bResult3 = TestEqual(TEXT("Accepting a request should publish Pending as the transition status"),
		Comp->GetTransitionStatus(), ESurfaceTransitionStatus::Pending);

	TestWorld.TickWorld(1/60.f);
	Comp->ExecuteReadPhase();

	const bool bResult4 = TestEqual(TEXT("Read should accept the pending request and publish Active"),
		Comp->GetTransitionStatus(), ESurfaceTransitionStatus::Active);

	const bool bResult5 = TestTrue(TEXT("The maneuver should reach its endpoint and await the arrival repin within the iteration cap"),
		TickUntilAwaitingArrivalRepin(TestWorld,Comp));

	const bool bResult6 = TestEqual(TEXT("Reaching the endpoint transform is not completion; status must remain Active"),
		Comp->GetTransitionStatus(), ESurfaceTransitionStatus::Active);
	const bool bResult7 = TestEqual(TEXT("Reaching the endpoint transform must not leave Transitioning; the repin has not run"),
		Comp->GetMovementMode(),ESurfaceMovementMode::Transitioning);
	const bool bResult8 = TestNotNull(TEXT("An accepted request should produce an active transition snapshot"),
		FSurfaceTransitionTestAccess::GetActiveTransition(Comp));
	const bool bResult9 = TestEqual(TEXT("The arc must never republish the committed surface; it stays the departure normal until the repin"),
		Comp->GetCommittedState().SurfaceNormal, DepartureNormal);

	const FVector EndLocation = Mover->GetActorLocation();

	TestWorld.TickWorld(1/60.f);
	Comp->ExecuteReadPhase();

	const bool bResult10 = TestEqual(TEXT("The arrival Read must not publish a terminal status; completion belongs to the repin Commit"),
		Comp->GetTransitionStatus(), ESurfaceTransitionStatus::Active);
	const bool bResult11 = TestEqual(TEXT("The arrival Read writes only the pending probe; the committed surface is still the departure normal"),
		Comp->GetCommittedState().SurfaceNormal, DepartureNormal);
	const bool bResult12 = TestNotEqual(TEXT("The arrival probe should trace along the frozen arrival normal and find the destination surface, not re-probe the departure surface"),
		Comp->GetPendingProbeResult().SurfaceNormal, DepartureNormal);

	Comp->ExecuteSimulatePhase();

	const bool bResult13 = TestEqual(TEXT("Simulate must not publish a terminal status on the repin frame"),
		Comp->GetTransitionStatus(), ESurfaceTransitionStatus::Active);
	const bool bResult14 = TestEqual(TEXT("Simulate must propose no movement on the repin frame; the endpoint transform is already committed"),
		Mover->GetActorLocation(), EndLocation);

	Comp->ExecuteCommitPhase();

	const bool bResult15 = TestEqual(TEXT("The repin Commit should publish Completed"),
		Comp->GetTransitionStatus(), ESurfaceTransitionStatus::Completed);
	const bool bResult16 = TestEqual(TEXT("A successful repin should return the component to Crawling"),
		Comp->GetMovementMode(), ESurfaceMovementMode::Crawling);
	const bool bResult17 = TestNull(TEXT("A successful repin must clear the active transition snapshot so later requests can be admitted"),
		FSurfaceTransitionTestAccess::GetActiveTransition(Comp));
	const bool bResult18 = TestTrue(TEXT("A successful repin should publish the destination surface as committed attachment"),
		Comp->GetCommittedState().bIsOnSurface);
	const bool bResult19 = TestEqual(TEXT("A successful repin should replace the departure normal with the arrival surface's hit normal"),
		Comp->GetCommittedState().SurfaceNormal, Arrival.TransitionInfo->ArrivalNormal, KINDA_SMALL_NUMBER);
	const bool bResult20 = TestEqual(TEXT("A successful repin should publish the probe's actual impact point on the arrival surface"),
		Comp->GetCommittedState().ImpactPoint, FVector(290,0,50), 0.1f);
	const bool bResult21 = TestEqual(TEXT("The repin Commit publishes probe evidence and must not move the actor"),
		Mover->GetActorLocation(), EndLocation);
	const bool bResult22 = TestTrue(TEXT("A completed transition must leave the component able to admit a new request"),
		Comp->RequestTransition(Arrival.Position, Arrival.TransitionInfo.GetValue()));

	return bResult1 && bResult2 && bResult3 && bResult4 && bResult5 && bResult6 && bResult7 && bResult8 && bResult9 &&
		bResult10 && bResult11 && bResult12 && bResult13 && bResult14 && bResult15 && bResult16 && bResult17 &&
		bResult18 && bResult19 && bResult20 && bResult21 && bResult22;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FSurfaceTransitionFailedArrivalRepin, "SurfaceNavigation.Movement.Transition.FailedArrivalRepin",
								 EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)
bool FSurfaceTransitionFailedArrivalRepin::RunTest(const FString& Parameters)
{
	const FSurfaceMovementTestWorld TestWorld = FSurfaceMovementTestWorld();
	const FTransitionFixture Fixture = BuildQuadraticReorientationFixture(TestWorld, false);

	USurfaceMovementComponent* Comp = Fixture.MovementComponent;
	const AActor* Mover = Fixture.MovementActor;

	const FSurfaceWaypoint Arrival = Fixture.Path.Waypoints[1];

	const bool bResult1 = TestTrue(TEXT("Movement component should reach the surface and start crawling before the transition is requested"),
			PrimeCrawlingAttachment(TestWorld, Comp));

	const FVector DepartureNormal = Comp->GetCommittedState().SurfaceNormal;
	const FVector DepartureImpactPoint = Comp->GetCommittedState().ImpactPoint;

	const bool bResult2 = TestTrue(TEXT("RequestTransition should accept a request when no transition is pending or active"),
		Comp->RequestTransition(Arrival.Position, Arrival.TransitionInfo.GetValue()));
	const bool bResult3 = TestTrue(TEXT("The maneuver should reach its endpoint and await the arrival repin within the iteration cap"),
			TickUntilAwaitingArrivalRepin(TestWorld,Comp));

	Comp->SetMovementTarget(FVector(1000,1000,0));

	const FVector EndLocation = Mover->GetActorLocation();
	const FRotator EndRotation = Mover->GetActorRotation();

	TestWorld.TickWorld(1/60.f);
	Comp->ExecuteReadPhase();

	const bool bResult4 = TestEqual(TEXT("The arrival Read must not publish a terminal status, even when the probe finds no geometry"),
		Comp->GetTransitionStatus(), ESurfaceTransitionStatus::Active);
	const bool bResult5 = TestFalse(TEXT("The arrival probe should report no surface when the destination geometry is absent"),
		Comp->GetPendingProbeResult().bIsOnSurface);

	Comp->ExecuteSimulatePhase();
	Comp->ExecuteCommitPhase();

	const bool bResult6 = TestEqual(TEXT("A failed repin should publish FailedArrivalRepin"),
		Comp->GetTransitionStatus(), ESurfaceTransitionStatus::FailedArrivalRepin);
	const bool bResult7 = TestEqual(TEXT("A failed repin should hand the actor to Falling and the ordinary recovery route"),
		Comp->GetMovementMode(), ESurfaceMovementMode::Falling);
	const bool bResult8 = TestNull(TEXT("A failed repin must clear the active transition snapshot so later requests can be admitted"),
		FSurfaceTransitionTestAccess::GetActiveTransition(Comp));
	const bool bResult9 = TestEqual(TEXT("A failed repin must preserve the last verified departure normal, not publish the missed probe's zero vector"),
		Comp->GetCommittedState().SurfaceNormal, DepartureNormal);
	const bool bResult10 = TestEqual(TEXT("A failed repin must preserve the last verified departure impact point"),
		Comp->GetCommittedState().ImpactPoint, DepartureImpactPoint);
	const bool bResult11 = TestTrue(TEXT("A failed repin leaves the last verified surface evidence intact for the next frame's recovery probe"),
		Comp->GetCommittedState().bIsOnSurface);
	const bool bResult12 = TestEqual(TEXT("A failed repin must not move the actor"),
		Mover->GetActorLocation(), EndLocation);
	const bool bResult13 = TestEqual(TEXT("A failed repin must not rotate the actor"),
		Mover->GetActorRotation(),EndRotation);

	for (int32 i = 0; i < 4; i++)
	{
		TestWorld.TickWorld(1/60.f);
		Comp->ExecuteReadPhase();
		Comp->ExecuteSimulatePhase();
		Comp->ExecuteCommitPhase();
	}

	const bool bResult14 = TestEqual(TEXT("A failed landing must leave the actor where the arc ended; Falling produces no movement"),
		Mover->GetActorLocation(), EndLocation);
	const bool bResult15 = TestEqual(TEXT("Transition status is level-held; it stays FailedArrivalRepin until another request is admitted"),
		Comp->GetTransitionStatus(), ESurfaceTransitionStatus::FailedArrivalRepin);
	const bool bResult16 = TestEqual(TEXT("A failed landing should stay in Falling while the recovery probe finds no surface"),
		Comp->GetMovementMode(), ESurfaceMovementMode::Falling);
	const bool bResult17 = TestEqual(TEXT("The probe's miss fallback should keep preserving the departure normal on later frames"),
		Comp->GetCommittedState().SurfaceNormal, DepartureNormal);
	const bool bResult18 = TestFalse(TEXT("A failed repin must clear the successor target and recovery frames must not resurrect it"),
		FSurfaceTransitionTestAccess::HasPendingTarget(Comp));

	return bResult1 && bResult2 && bResult3 && bResult4 && bResult5 && bResult6 && bResult7 && bResult8 && bResult9 &&
		bResult10 && bResult11 && bResult12 && bResult13 && bResult14 && bResult15 && bResult16 && bResult17 && bResult18;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FSurfaceTransitionSuccessorTargetHandoff, "SurfaceNavigation.Movement.Transition.SuccessorTargetHandoff",
								 EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)
bool FSurfaceTransitionSuccessorTargetHandoff::RunTest(const FString& Parameters)
{
	const FSurfaceMovementTestWorld TestWorld = FSurfaceMovementTestWorld();
	const FTransitionFixture Fixture = BuildQuadraticReorientationFixture(TestWorld, true);

	USurfaceMovementComponent* Comp = Fixture.MovementComponent;
	const AActor* Mover = Fixture.MovementActor;

	const FSurfaceWaypoint Arrival = Fixture.Path.Waypoints[1];

	const FVector PreRequestTarget = FVector(50, 300, 50);
	const FVector SuccessorA = FVector(250, -500, 50);
	const FVector SuccessorB = FVector(250, 500, 50);

	const bool bResult1 = TestTrue(TEXT("Movement component should reach the surface and start crawling before the transition is requested"),
		PrimeCrawlingAttachment(TestWorld, Comp));

	const FVector DepartureLocation = Mover->GetActorLocation();

	Comp->SetMovementTarget(PreRequestTarget);

	const bool bResult2 = TestTrue(TEXT("A target published before the request should occupy the single target slot"),
		FSurfaceTransitionTestAccess::HasPendingTarget(Comp));
	const bool bResult3 = TestTrue(TEXT("RequestTransition should accept a request when no transition is pending or active"),
		Comp->RequestTransition(Arrival.Position, Arrival.TransitionInfo.GetValue()));
	const bool bResult4 = TestEqual(TEXT("Accepting a request should publish Pending as the transition status"),
		Comp->GetTransitionStatus(), ESurfaceTransitionStatus::Pending);

	TestWorld.TickWorld(1/60.f);
	Comp->ExecuteReadPhase();

	const bool bResult5 = TestEqual(TEXT("Read should accept the pending request and publish Active"),
		Comp->GetTransitionStatus(), ESurfaceTransitionStatus::Active);
	const bool bResult6 = TestFalse(TEXT("Acceptance should clear the departure target; a target published before acceptance is not a successor"),
		FSurfaceTransitionTestAccess::HasPendingTarget(Comp));

	Comp->SetMovementTarget(SuccessorA);

	const bool bResult7 = TestTrue(TEXT("SetMovementTarget should be admitted while a transition is active; the slot is not gated on movement mode"),
		FSurfaceTransitionTestAccess::HasPendingTarget(Comp));
	const bool bResult8 = TestEqual(TEXT("A successor published during an active transition should occupy the target slot"),
		FSurfaceTransitionTestAccess::GetPendingTarget(Comp), SuccessorA);

	Comp->SetMovementTarget(SuccessorB);

	const bool bResult9 = TestEqual(TEXT("A second successor should replace the first; the slot holds one latest target and never a queue"),
		FSurfaceTransitionTestAccess::GetPendingTarget(Comp), SuccessorB);

	Comp->ExecuteSimulatePhase();
	Comp->ExecuteCommitPhase();

	const bool bResult10 = TestEqual(TEXT("Simulate should ignore the successor while transitioning; the arc never leaves the departure-to-arrival plane"),
		Mover->GetActorLocation().Y, 0.0, 0.1);
	const bool bResult11 = TestNotEqual(TEXT("The traversal should still advance along the arc, so the unchanged-axis assertion cannot pass on an inert frame"),
		Mover->GetActorLocation(), DepartureLocation);
	const bool bResult12 = TestTrue(TEXT("The maneuver should reach its endpoint and await the arrival repin within the iteration cap"),
		TickUntilAwaitingArrivalRepin(TestWorld, Comp));
	const bool bResult13 = TestTrue(TEXT("The successor should survive the entire traversal; ignoring a target must not destroy it"),
		FSurfaceTransitionTestAccess::HasPendingTarget(Comp));
	const bool bResult14 = TestEqual(TEXT("A completed arc should leave the actor in the departure-to-arrival plane, never steered toward the successor"),
		Mover->GetActorLocation().Y, 0.0, 0.1);

	const FVector EndLocation = Mover->GetActorLocation();

	TestWorld.TickWorld(1/60.f);
	Comp->ExecuteReadPhase();
	Comp->ExecuteSimulatePhase();
	Comp->ExecuteCommitPhase();

	const bool bResult15 = TestEqual(TEXT("A successful repin should publish Completed"),
		Comp->GetTransitionStatus(), ESurfaceTransitionStatus::Completed);
	const bool bResult16 = TestEqual(TEXT("A successful repin should hand the actor back to Crawling"),
		Comp->GetMovementMode(), ESurfaceMovementMode::Crawling);
	const bool bResult17 = TestEqual(TEXT("The repin frame must produce no ordinary movement; Simulate returned early while awaiting the repin"),
		Mover->GetActorLocation(), EndLocation);
	const bool bResult18 = TestTrue(TEXT("A successor published after acceptance should be retained across completion"),
		FSurfaceTransitionTestAccess::HasPendingTarget(Comp));
	const bool bResult19 = TestEqual(TEXT("Completion should retain the latest successor, not resurrect the one it replaced"),
		FSurfaceTransitionTestAccess::GetPendingTarget(Comp), SuccessorB);

	const double Distance = FVector::Dist(Mover->GetActorLocation(), SuccessorB);

	TestWorld.TickWorld(1/60.f);
	Comp->ExecuteReadPhase();
	Comp->ExecuteSimulatePhase();
	Comp->ExecuteCommitPhase();

	const bool bResult20 = TestLessThan(TEXT("Crawling should resume toward the successor on the first full RSC frame after completion"),
		FVector::Dist(Mover->GetActorLocation(), SuccessorB), Distance);
	const bool bResult21 = TestGreaterThan(TEXT("Resumed crawling should move toward the retained successor, not the replaced one"),
		Mover->GetActorLocation().Y, EndLocation.Y);

	return bResult1 && bResult2 && bResult3 && bResult4 && bResult5 && bResult6 && bResult7 && bResult8 && bResult9 &&
		bResult10 && bResult11 && bResult12 && bResult13 && bResult14 && bResult15 && bResult16 && bResult17 &&
		bResult18 && bResult19 && bResult20 && bResult21;
}

#endif