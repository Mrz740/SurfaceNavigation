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
	
	int32 OutTriangleID;
	const bool bResult16 = TestTrue(TEXT("Point strictly inside a triangle should report success"), Adapter.FindNearestTriangle(FVector(10,6,0), OutTriangleID));
	const bool bResult17 = TestEqual(TEXT("Point strictly inside the center triangle should resolve to it, and finding the highest-ordinal triangle proves the scan does not simply return the first"), OutTriangleID, 3);

	const bool bResult18 = TestTrue(TEXT("Point on a shared triangle edge should report success"), Adapter.FindNearestTriangle(FVector(6,9,0), OutTriangleID));
	const bool bResult19 = TestEqual(TEXT("Point exactly on the edge shared by corner triangle 2 and the center triangle should resolve to the lower ordinal, since both report containment"), OutTriangleID, 2);

	const bool bResult20 = TestTrue(TEXT("Point outside every triangle should still report success, since the fallback always resolves to some triangle"), Adapter.FindNearestTriangle(FVector(30,0,50), OutTriangleID));
	const bool bResult21 = TestEqual(TEXT("Point outside every triangle's projected footprint should fall back to the nearest centroid, which is corner triangle 1, and an off-plane Z should not prevent that"), OutTriangleID, 1);

	const bool bResult22 = TestTrue(TEXT("Point far above a triangle should report success"), Adapter.FindNearestTriangle(FVector(10,6,100), OutTriangleID));
	const bool bResult23 = TestEqual(TEXT("Containment is tested in the projected plane basis, so a point far above a triangle still resolves to it rather than falling back"), OutTriangleID, 3);

	const bool bResult24 = TestTrue(TEXT("Extremely distant point should report success"), Adapter.FindNearestTriangle(FVector(200000,0,0), OutTriangleID));
	const bool bResult25 = TestEqual(TEXT("A far-off-surface point must still resolve by nearest centroid; squared distances at this range exceed 32-bit integer capacity, so this fails if the accumulator is not floating point"), OutTriangleID, 1);

	const int32 EmptyNodeIndex = AddEmptyNode(*Fixture.Graph);
	const FNodeTriangleAStarAdapter EmptyAdapter(*Fixture.Graph, EmptyNodeIndex);
	
	const bool bResult26 = TestFalse(TEXT("An adapter over a node with no triangles should report failure rather than resolving to an arbitrary triangle"),EmptyAdapter.FindNearestTriangle(FVector(10,6,100), OutTriangleID));
	const bool bResult27 = TestEqual(TEXT("A failed lookup should reset the out-parameter to INDEX_NONE rather than leaving the previous call's triangle ID in place"), OutTriangleID, INDEX_NONE);
	
	TGuardValue SuppressEnsureErrors(FAutomationTestBase::bSuppressLogErrors, true);
	const FNodeTriangleAStarAdapter InvalidAdapter(*Fixture.Graph, 999);

	const bool bResult28 = TestFalse(TEXT("An adapter constructed on an invalid node index should degrade to reporting no triangles, as its constructor's diagnostic promises, rather than asserting at query time"), InvalidAdapter.FindNearestTriangle(FVector(10,0,100), OutTriangleID));

	return bResult1 && bResult2 && bResult3 && bResult4 && bResult5 && bResult6 && bResult7 && bResult8 && bResult9
	&& bResult10 && bResult11 && bResult12 && bResult13 && bResult14 && bResult15 && bResult16 && bResult17
	&& bResult18 && bResult19 && bResult20 && bResult21 && bResult22 && bResult23 && bResult24 && bResult25
	&& bResult26 && bResult27 && bResult28;
}

#endif // WITH_DEV_AUTOMATION_TESTS