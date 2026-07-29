#include "Misc/AutomationTest.h"
#include "SurfaceGraphTestUtils.h"
#include "Pathfinding/NodeTriangleAStarAdapter.h"

#if WITH_DEV_AUTOMATION_TESTS

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FNodeTriangleAStarAdapterTests, "SurfaceNavigation.Pathfinding.NodeTriangleAStarAdapter",
                                 EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)

bool FNodeTriangleAStarAdapterTests::RunTest(const FString& Parameters)
{
	using namespace SurfaceGraphTestUtils;

	const FMultiTriangleNodeFixture Fixture = BuildMultiTriangleNodeGraph();
	const FNodeTriangleAStarAdapter Adapter(*Fixture.Graph, Fixture.NodeIndex);
	TArray<int32> Neighbors;

	Adapter.GetNeighbors(Fixture.CenterTriangleIndex, Neighbors);
	const bool bResult1 = TestEqual(TEXT("Center triangle should have exactly three neighbors, since it shares an edge with all three corner triangles"), Neighbors.Num(),3);
	const bool bResult2 = TestTrue(TEXT("Center triangle's neighbors should include corner triangle 0"), Neighbors.Contains(Fixture.CornerTriangleIndex0));
	const bool bResult3 = TestTrue(TEXT("Center triangle's neighbors should include corner triangle 1"), Neighbors.Contains(Fixture.CornerTriangleIndex1));
	const bool bResult4 = TestTrue(TEXT("Center triangle's neighbors should include corner triangle 2"), Neighbors.Contains(Fixture.CornerTriangleIndex2));

	Adapter.GetNeighbors(Fixture.CornerTriangleIndex0, Neighbors);
	const bool bResult5 = TestEqual(TEXT("Corner triangle 0 should have exactly one neighbor, since only its shared edge with the center triangle is internal"), Neighbors.Num(),1);
	const bool bResult6 = TestTrue(TEXT("Corner triangle 0's only neighbor should be the center triangle"), Neighbors.Contains(Fixture.CenterTriangleIndex));

	Adapter.GetNeighbors(Fixture.CornerTriangleIndex1, Neighbors);
	const bool bResult7 = TestEqual(TEXT("Corner triangle 1 should have exactly one neighbor, since only its shared edge with the center triangle is internal"), Neighbors.Num(),1);
	const bool bResult8 = TestTrue(TEXT("Corner triangle 1's only neighbor should be the center triangle"), Neighbors.Contains(Fixture.CenterTriangleIndex));

	Adapter.GetNeighbors(Fixture.CornerTriangleIndex2, Neighbors);
	const bool bResult9 = TestEqual(TEXT("Corner triangle 2 should have exactly one neighbor, since only its shared edge with the center triangle is internal"), Neighbors.Num(),1);
	const bool bResult10 = TestTrue(TEXT("Corner triangle 2's only neighbor should be the center triangle"), Neighbors.Contains(Fixture.CenterTriangleIndex));

	Adapter.GetNeighbors(999, Neighbors);
	const bool bResult11 = TestEqual(TEXT("Out-of-range TriangleID should Reset the array to zero neighbors, not leave the previous call's results"), Neighbors.Num(),0);

	const float Cost1 = Adapter.GetCost(Fixture.CenterTriangleIndex, Fixture.CornerTriangleIndex0);
	const float Cost2 = Adapter.GetCost(Fixture.CornerTriangleIndex0, Fixture.CenterTriangleIndex);

	const bool bResult12 = TestEqual(TEXT("GetCost(Center, Corner0) should be the sum of center-to-midpoint and midpoint-to-corner distances across their shared edge"), Cost1, 5.0665f , KINDA_SMALL_NUMBER);
	const bool bResult13 = TestEqual(TEXT("GetCost should be direction-symmetric at this tier, unlike the macro adapter, since there is only one shared edge and one midpoint rather than two independent portal sides"), Cost1, Cost2, KINDA_SMALL_NUMBER);

	const bool bResult14 = TestEqual(TEXT("GetHeuristic(Corner0, Center) should equal straight-line centroid distance, which is deliberately not equal to GetCost in this fixture's asymmetric geometry"), Adapter.GetHeuristic(Fixture.CornerTriangleIndex0, Fixture.CenterTriangleIndex), 5.0f, KINDA_SMALL_NUMBER);
	const bool bResult15 = TestEqual(TEXT("GetHeuristic with an out-of-range TriangleID should return 0 rather than index out of bounds, since FindAStarPath may call it with unvalidated caller-supplied IDs"), Adapter.GetHeuristic(999, Fixture.CenterTriangleIndex), 0.0f, KINDA_SMALL_NUMBER);

	return bResult1 && bResult2 && bResult3 && bResult4 && bResult5 && bResult6 && bResult7 && bResult8 && bResult9 
	&& bResult10 && bResult11 && bResult12 && bResult13 && bResult14 && bResult15;
}

#endif // WITH_DEV_AUTOMATION_TESTS