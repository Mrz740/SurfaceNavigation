#include "Misc/AutomationTest.h"
#include "SurfaceGraphTestUtils.h"
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

	return bResult1 && bResult2 && bResult3 && bResult4 && bResult5;
}

#endif // WITH_DEV_AUTOMATION_TESTS