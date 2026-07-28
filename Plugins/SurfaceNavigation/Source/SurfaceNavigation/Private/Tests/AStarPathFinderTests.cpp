#include "Misc/AutomationTest.h"
#include "PathFinding/AStarPathFinder.h"
#include "SurfaceGraphTestUtils.h"
#include "Pathfinding/SurfaceGraphAStarAdapter.h"

#if WITH_DEV_AUTOMATION_TESTS

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FAStarPathFinderTests, "SurfaceNavigation.Pathfinding.FindAStarPath",
                                 EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)

bool FAStarPathFinderTests::RunTest(const FString& Parameters)
{
	using namespace SurfaceGraphTestUtils;
	
	const FDecoyCycleGraphFixture Fixture = BuildDecoyCycleGraph();
	const int32 NodeIslandIndex = AddIsolatedNode(*Fixture.Graph);
	const FSurfaceGraphAStarAdapter Adapter(*Fixture.Graph);
	TArray<int32> OutPath;
	
	const bool bFound = FindAStarPath(Adapter, Fixture.NodeSIndex, Fixture.NodeGIndex, OutPath);
	
	const bool bResult1 = TestTrue(TEXT("FindAStarPath should return true: NodeS and NodeG are connected via the gap bridge"), bFound);
	const bool bResult2 = TestTrue(TEXT("OutPath should be exactly {NodeS, NodeG} \u2014 the ~14.5-cost direct bridge, not the ~24.3-cost four-hop chain through NodeM1-M3, proving optimality rather than mere connectivity"),
		OutPath == TArray{Fixture.NodeSIndex,Fixture.NodeGIndex});
	
	TArray<int32> OutPathDisconnected;

	const bool bFoundDisconnected = FindAStarPath(Adapter, Fixture.NodeSIndex, NodeIslandIndex, OutPathDisconnected);

	const bool bResult3 = TestTrue(TEXT("FindAStarPath should return false: NodeIsland has no edges, so it is unreachable from NodeS"), !bFoundDisconnected);
	const bool bResult4 = TestEqual(TEXT("OutPath should be left empty when no path is found, not holding a partial or stale path from a prior call"), OutPathDisconnected.Num(), 0);

	TArray<int32> OutPathSameNode;
	const bool bFoundSameNode = FindAStarPath(Adapter, Fixture.NodeSIndex, Fixture.NodeSIndex, OutPathSameNode);
	
	const bool bResult5 = TestTrue(TEXT("FindAStarPath should return true: NodeS to itself is the trivial same-node case, resolved on the very first pop before any neighbor is expanded"), bFoundSameNode);
	const bool bResult6 = TestEqual(TEXT("OutPath should be exactly {NodeS} — a single-element path, not an empty one, per the decided same-node contract"), OutPathSameNode, TArray{Fixture.NodeSIndex});

	TArray<int32> OutPathBadStart;
	const bool bFoundBadStart = FindAStarPath(Adapter, 999, Fixture.NodeGIndex, OutPathBadStart);

	const bool bResult7 = TestTrue(TEXT("FindAStarPath should return false: 999 is not a valid StartNodeID, and should fail safely rather than crash"), !bFoundBadStart);
	const bool bResult8 = TestEqual(TEXT("OutPath should be left empty when the start ID is invalid"), OutPathBadStart.Num(), 0);

	TArray<int32> OutPathBadGoal;
	const bool bFoundBadGoal = FindAStarPath(Adapter, Fixture.NodeSIndex, 999, OutPathBadGoal);

	const bool bResult9 = TestTrue(TEXT("FindAStarPath should return false: 999 is not a valid GoalNodeID, and should fail safely rather than crash"), !bFoundBadGoal);
	const bool bResult10 = TestEqual(TEXT("OutPath should be left empty when the goal ID is invalid"), OutPathBadGoal.Num(), 0);
	
	return bResult1 && bResult2 && bResult3 && bResult4 && bResult5 && bResult6 && bResult7 && bResult8 && bResult9 && bResult10;
}

#endif // WITH_DEV_AUTOMATION_TESTS