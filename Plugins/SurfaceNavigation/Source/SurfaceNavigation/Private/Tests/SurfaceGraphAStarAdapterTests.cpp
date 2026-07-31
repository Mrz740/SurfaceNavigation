#include "Misc/AutomationTest.h"
#include "Utils/SurfaceGraphTestUtils.h"
#include "Pathfinding/SurfaceGraphAStarAdapter.h"

#if WITH_DEV_AUTOMATION_TESTS

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FSurfaceGraphAStarAdapterTests, "SurfaceNavigation.Pathfinding.SurfaceGraphAStarAdapter",
                                 EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)

bool FSurfaceGraphAStarAdapterTests::RunTest(const FString& Parameters)
{
	using namespace SurfaceGraphTestUtils;
	
	const FTwoNodeDiagonalSquareFixture Fixture = BuildTwoNodeDiagonalSquareGraph();
	const FSurfaceGraphAStarAdapter Adapter(*Fixture.Graph);
	TArray<int32> Neighbors;
	
	Adapter.GetNeighbors(Fixture.NodeAIndex, Neighbors);
	const bool bResult1 = TestEqual(TEXT("NodeA should have exactly one neighbor"), Neighbors.Num(),1);
	const bool bResult2 = TestTrue(TEXT("NodeA's neighbor should be NodeB"), Neighbors.Contains(Fixture.NodeBIndex));

	Adapter.GetNeighbors(Fixture.NodeBIndex, Neighbors);
	const bool bResult3 = TestEqual(TEXT("NodeB should have exactly one neighbor"), Neighbors.Num(),1);
	const bool bResult4 = TestTrue(TEXT("NodeB's neighbor should be NodeA"), Neighbors.Contains(Fixture.NodeAIndex));

	Adapter.GetNeighbors(999,Neighbors);
	const bool bResult5 = TestEqual(TEXT("Out-of-range NodeID should Reset the array to zero neighbors, not leave the previous call's results"), Neighbors.Num(),0);
	
	const bool bResult6 = TestEqual(TEXT("GetCost(NodeA, NodeB) should be twice the center-to-portal-midpoint distance, since both sides' portal midpoints coincide at (5,5,0) in this symmetric fixture, collapsing the middle leg of the three-segment sum to zero"),Adapter.GetCost(Fixture.NodeAIndex, Fixture.NodeBIndex), 4.714f, KINDA_SMALL_NUMBER);
	const bool bResult7 = TestEqual(TEXT("GetCost(NodeB, NodeA) should match GetCost(NodeA, NodeB) by the fixture's symmetry, and exercises the adapter's NodeB-is-A-side branch instead of NodeA's"),Adapter.GetCost(Fixture.NodeBIndex, Fixture.NodeAIndex), 4.714f, KINDA_SMALL_NUMBER);
	const bool bResult8 = TestEqual(TEXT("GetHeuristic(NodeA, NodeB) should equal straight-line center-to-center distance, which coincidentally equals GetCost here only because this fixture's portal midpoint sits exactly on the line between both centers"),Adapter.GetHeuristic(Fixture.NodeAIndex, Fixture.NodeBIndex), 4.714f, KINDA_SMALL_NUMBER);
	
	return bResult1 && bResult2 && bResult3 && bResult4 && bResult5 && bResult6 && bResult7 && bResult8;
}

#endif // WITH_DEV_AUTOMATION_TESTS