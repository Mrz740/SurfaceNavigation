#include "Misc/AutomationTest.h"
#include "Utils/SurfaceGraphTestUtils.h"
#include "Pathfinding/NodeTriangleAStarAdapter.h"
#include "Pathfinding/AStarPathFinder.h"
#include "Pathfinding/SurfacePathFinder.h"

#if WITH_DEV_AUTOMATION_TESTS

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FSurfacePathFinderTests, "SurfaceNavigation.Pathfinding.FindSurfacePath",
                                 EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)

bool FSurfacePathFinderTests::RunTest(const FString& Parameters)
{
	const FThreeNodeChainFixture Fixture = SurfaceGraphTestUtils::BuildThreeNodeChainGraph();
	FSurfacePath OutPath;

	const FVector StartPosition = FVector(4,12,0);
	const FVector GoalPosition = FVector(40,18,7);
	const int32 StartNodeIndex = Fixture.Node1Index;
	const int32 GoalNodeIndex = Fixture.Node3Index;
	
	const bool bFoundPath = FindSurfacePath(*Fixture.Graph, StartPosition, StartNodeIndex, GoalPosition, GoalNodeIndex,OutPath);
	
	const bool bResult1 = TestTrue(TEXT("FindSurfacePath should succeed for a start/goal pair spanning all three chained nodes"), bFoundPath);
	const bool bResult2 = TestEqual(TEXT("FindSurfacePath should emit exactly 11 waypoints for this three-node chain"), OutPath.Waypoints.Num(), 11);

	if (!bResult1) return false;

	auto CheckWaypoint = [this, &OutPath](const int32 Index, const FVector& ExpectedPosition, const int32 ExpectedNodeIndex) -> bool
	{
		const bool bPos  = TestEqual(*FString::Printf(TEXT("Waypoint %d should have the expected position"), Index),
			OutPath.Waypoints[Index].Position, ExpectedPosition);
		const bool bNode = TestEqual(*FString::Printf(TEXT("Waypoint %d should be on the expected node"), Index),
			OutPath.Waypoints[Index].NodeIndex, ExpectedNodeIndex);
		const bool bUnset = TestFalse(*FString::Printf(TEXT("Waypoint %d should carry no TransitionInfo, since it "
													  "is not a reorientation or gap-bridge crossing"), Index), 
													  OutPath.Waypoints[Index].TransitionInfo.IsSet());
		return bPos && bNode && bUnset;
	};
	
	const bool bResult3 = CheckWaypoint(0, FVector(4,12,0), Fixture.Node1Index);
	const bool bResult4 = CheckWaypoint(1, FVector(6,9,0), Fixture.Node1Index);
	const bool bResult5 = CheckWaypoint(2, FVector(8,4.5,0), Fixture.Node1Index);
	const bool bResult6 = CheckWaypoint(3, FVector(8,0,0), Fixture.Node1Index);
	const bool bResult7 = CheckWaypoint(4, FVector(8,0,0), Fixture.Node2Index);
	const bool bResult8 = CheckWaypoint(5, FVector(8,-10,0), Fixture.Node2Index);
	const bool bResult9 = CheckWaypoint(6, FVector(8,-20,0), Fixture.Node2Index);

	const bool bResult10 = TestEqual(TEXT("Waypoint 7 (entry onto Node3 across the gap-bridge portal) should have the expected position"),
		OutPath.Waypoints[7].Position, FVector(40,-2,6));
	const bool bResult11 = TestEqual(TEXT("Waypoint 7 should be on Node3"), OutPath.Waypoints[7].NodeIndex, 
		Fixture.Node3Index);
	const bool bResult12 = TestTrue(TEXT("Waypoint 7 should carry a TransitionInfo, since it is a reorientation and gap-bridge crossing"),
		OutPath.Waypoints[7].TransitionInfo.IsSet());
	const bool bResult13 = TestEqual(TEXT("Waypoint 7's TransitionInfo.DepartureNormal should match Node2's normal"),
		OutPath.Waypoints[7].TransitionInfo->DepartureNormal, FVector(0,0,1));
	const bool bResult14 = TestEqual(TEXT("Waypoint 7's TransitionInfo.ArrivalNormal should match Node3's normal"),
		OutPath.Waypoints[7].TransitionInfo->ArrivalNormal, FVector(1,0,0));
	const bool bResult15 = TestTrue(TEXT("Waypoint 7's TransitionInfo.bRequiresReorientation should be true, matching the edge's flag"), 
		OutPath.Waypoints[7].TransitionInfo->bRequiresReorientation);
	const bool bResult16 = TestTrue(TEXT("Waypoint 7's TransitionInfo.bIsGapBridge should be true, matching the edge's flag"),
		OutPath.Waypoints[7].TransitionInfo->bIsGapBridge);

	const bool bResult17 = CheckWaypoint(8, FVector(40,5,5), Fixture.Node3Index);
	const bool bResult18 = CheckWaypoint(9, FVector(40,15,5), Fixture.Node3Index);
	const bool bResult19 = CheckWaypoint(10, FVector(40,18,7), Fixture.Node3Index);

	return bResult1 && bResult2 && bResult3 && bResult4 && bResult5 && bResult6 && bResult7 && bResult8 && bResult9
	&& bResult10 && bResult11 && bResult12 && bResult13 && bResult14 && bResult15 && bResult16 && bResult17 && bResult18
	&& bResult19;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FLocalConfinementTests, "SurfaceNavigation.Pathfinding.LocalConfinement",
                                 EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)

bool FLocalConfinementTests::RunTest(const FString& Parameters)
{
	const FThreeNodeChainFixture Fixture = SurfaceGraphTestUtils::BuildThreeNodeChainGraph();

	const FNodeTriangleAStarAdapter Node3Adapter(*Fixture.Graph, Fixture.Node3Index);
	TArray<int32> Node3TriSequence;
	const bool bFoundNode3Path = FindAStarPath(Node3Adapter, 3, 1, Node3TriSequence);
	
	const bool bResult1 = TestTrue(TEXT("FindAStarPath should succeed for triangles 3 and 1 within Node3's local adapter"), bFoundNode3Path);
	bool bResult2 = true;
	for (int32 i = 0; i < Node3TriSequence.Num(); i++)
	{
		bResult2 = TestTrue(*FString::Printf(TEXT("Node3's local path entry %d should be a valid triangle ordinal "
											"within Node3's own range [0,4]"), i), Node3TriSequence[i] >= 0 && Node3TriSequence[i] <= 4) && bResult2;
	}

	const FNodeTriangleAStarAdapter Node2Adapter(*Fixture.Graph, Fixture.Node2Index);
	TArray<int32> Node2TriSequence;
	const bool bFoundNode2Path = FindAStarPath(Node2Adapter, 0, 1, Node2TriSequence);
	
	const bool bResult3 = TestTrue(TEXT("FindAStarPath should succeed for triangles 0 and 1 within Node2's local adapter"), bFoundNode2Path);
	bool bResult4 = true;
	for (int32 i = 0; i < Node2TriSequence.Num(); i++)
	{
		bResult4 = TestTrue(*FString::Printf(TEXT("Node2's local path entry %d should be a valid triangle ordinal "
											"within Node2's own range [0,1] — a value >= 2 would mean a triangle leaked in from another node's adapter"), i), Node2TriSequence[i] >= 0 && Node2TriSequence[i] <= 1) && bResult4;
	}
	
	return bResult1 && bResult2 && bResult3 && bResult4;
}

#endif // WITH_DEV_AUTOMATION_TESTS
