#include "Utils/SurfaceGraphTestUtils.h"
#include "Misc/AutomationTest.h"
#include "Pathfinding/SurfacePathFinder.h"

#if WITH_DEV_AUTOMATION_TESTS

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FSurfacePathFinderRegressionTests, "SurfaceNavigation.Pathfinding.FindSurfacePathRegressions",
                                 EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)

bool FSurfacePathFinderRegressionTests::RunTest(const FString& Parameters)
{
	const FThreeNodeChainFixture Fixture = SurfaceGraphTestUtils::BuildThreeNodeChainGraph();
	FSurfacePath OutPath;
	
	const bool bFoundSameNodePath = FindSurfacePath(*Fixture.Graph, FVector(4,12,0),
		Fixture.Node1Index, FVector(2,2,0), Fixture.Node1Index, OutPath);
	
	const bool bResult1 = TestTrue(TEXT("FindSurfacePath should succeed when start and goal lie on the same node"), bFoundSameNodePath);
	const bool bResult2 = TestEqual(TEXT("A same-node request through Node1's triangles should emit exactly 4 waypoints"), OutPath.Waypoints.Num(), 4);
	bool bResult3 = true;
	bool bResult4 = true;

	for (int32 i = 0; i < OutPath.Waypoints.Num(); i++)
	{
		bResult3 = TestTrue(*FString::Printf(TEXT("Same-node waypoint %d should be on Node1, the only node in the request"), i), OutPath.Waypoints[i].NodeIndex == Fixture.Node1Index) && bResult3;
		bResult4 = TestFalse(*FString::Printf(TEXT("Same-node waypoint %d should carry no TransitionInfo, since no portal is crossed"), i), OutPath.Waypoints[i].TransitionInfo.IsSet()) && bResult4;
	}
	
	const int32 IsolatedNodeIndex = SurfaceGraphTestUtils::AddIsolatedNode(*Fixture.Graph);
	const bool bFoundDisconnectedPath = FindSurfacePath(*Fixture.Graph, FVector(4,12,0), Fixture.Node1Index,
														FVector(105,105,0), IsolatedNodeIndex, OutPath);
	const bool bResult5 = TestFalse(TEXT("FindSurfacePath should fail when the goal node has no portal connecting it to the rest of the graph"), bFoundDisconnectedPath);
	const bool bResult6 = TestEqual(TEXT("A failed request should leave the path empty, resetting the waypoints the previous request filled in"), OutPath.Waypoints.Num(), 0);

	const bool bFoundBogusPath = FindSurfacePath(*Fixture.Graph, FVector(4,12,0), Fixture.Node1Index,
											 FVector(40,18,7), 999, OutPath);
	
	const bool bResult7 = TestFalse(TEXT("FindSurfacePath should fail cleanly for an out-of-range goal node index"), bFoundBogusPath);
	const bool bResult8 = TestEqual(TEXT("An out-of-range request should leave the path empty"), OutPath.Waypoints.Num(), 0);
	
	return bResult1 && bResult2 && bResult3 && bResult4 && bResult5 && bResult6 && bResult7 && bResult8;
}

#endif // WITH_DEV_AUTOMATION_TESTS
