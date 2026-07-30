#include "Misc/AutomationTest.h"
#include "SurfaceGraphTestUtils.h"

#if WITH_DEV_AUTOMATION_TESTS

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FSurfaceGraphTests, "SurfaceNavigation.SurfaceGraph.TwoNodePortalGraph",
                                 EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)

bool FSurfaceGraphTests::RunTest(const FString& Parameters)
{
	using namespace SurfaceGraphTestUtils;

	const FTwoNodeDiagonalSquareFixture Fixture = BuildTwoNodeDiagonalSquareGraph();

	const FSurfaceGraphNode* NodeAPtr = Fixture.Graph->GetNode(Fixture.NodeAIndex);
	const FSurfaceGraphNode* NodeBPtr = Fixture.Graph->GetNode(Fixture.NodeBIndex);
	const FSurfaceGraphEdge* EdgePtr = Fixture.Graph->GetEdge(Fixture.EdgeIndex);
	
	const bool bResult1 = TestEqual(TEXT("Edge should reference NodeA by index"), EdgePtr->NodeAIndex,Fixture.NodeAIndex);
	const bool bResult2 = TestEqual(TEXT("Edge should reference NodeB by index"), EdgePtr->NodeBIndex,Fixture.NodeBIndex);
	const bool bResult3 = TestTrue(TEXT("NodeA's NeighbourEdgeIndices should contain the shared edge"), NodeAPtr->NeighborEdgeIndices.Contains(Fixture.EdgeIndex));
	const bool bResult4 = TestTrue(TEXT("NodeB's NeighbourEdgeIndices should contain the shared edge"), NodeBPtr->NeighborEdgeIndices.Contains(Fixture.EdgeIndex));
	const bool bResult5 = TestEqual(TEXT("Shared vertex pool should hold exactly 4 entries, not 8"),Fixture.Graph->GetSharedVertexCount(),4);
	const bool bResult6 = TestTrue(TEXT("NodeA's boundary should reference the shared corner index"),NodeAPtr->OuterBoundaryIndices.Contains(Fixture.Idx0));
	const bool bResult7 = TestTrue(TEXT("NodeB's boundary should reference the shared corner index"),NodeBPtr->OuterBoundaryIndices.Contains(Fixture.Idx0));
	
	const FVector InsidePoint = FVector(7,2,0);
	const FVector OutsidePoint = FVector(2,7,0);
	const FVector BoundaryPoint = FVector(5,5,0);
	
	const bool bResult8 = TestTrue(TEXT("Point strictly inside NodeA should return true"),Fixture.Graph->IsPointInPolygon(*NodeAPtr,InsidePoint));
	const bool bResult9 = TestFalse(TEXT("Point strictly outside NodeA should return false"),Fixture.Graph->IsPointInPolygon(*NodeAPtr,OutsidePoint));
	const bool bResult10 = TestTrue(TEXT("Point exactly on the shared boundary edge should return true"),Fixture.Graph->IsPointInPolygon(*NodeAPtr,BoundaryPoint));
	
	const FSurfaceGraphNode* InvalidNodePtr = Fixture.Graph->GetNode(999);
	const FSurfaceGraphEdge* InvalidEdgePtr = Fixture.Graph->GetEdge(999);
	const FSurfaceGraphNode* NegativeNodePtr = Fixture.Graph->GetNode(-1);

	const bool bResult11 = TestNull(TEXT("Out-of-range node index should return nullptr, not crash"),InvalidNodePtr);
	const bool bResult12 = TestNull(TEXT("Out-of-range edge index should return nullptr, not crash"),InvalidEdgePtr);
	const bool bResult13 = TestNull(TEXT("Negative node index should return nullptr, not crash"),NegativeNodePtr);

	const int32 SlotA = EdgePtr->PortalSlotA;
	const int32 CountA = NodeAPtr->OuterBoundaryIndices.Num();

	const bool bResult14 = TestTrue(TEXT("NodeA's portal slot should be a valid index into its outer boundary"),NodeAPtr->OuterBoundaryIndices.IsValidIndex(SlotA));
	bool bResult15 = false;
	bool bResult16 = false;
	if (bResult14)
	{
		const int32 First = NodeAPtr->OuterBoundaryIndices[SlotA];
		const int32 Second = NodeAPtr->OuterBoundaryIndices[(SlotA + 1) % CountA];
		
		bResult15 = TestTrue(TEXT("NodeA's portal slot should resolve to a boundary edge containing shared vertex Idx0"), First == Fixture.Idx0 || Second == Fixture.Idx0);
		bResult16 = TestTrue(TEXT("NodeA's portal slot should resolve to a boundary edge containing shared vertex Idx2"), First == Fixture.Idx2 || Second == Fixture.Idx2);
	}

	const int32 SlotB = EdgePtr->PortalSlotB;
	const int32 CountB = NodeBPtr->OuterBoundaryIndices.Num();

	const bool bResult17 = TestTrue(TEXT("NodeB's portal slot should be a valid index into its outer boundary"),NodeBPtr->OuterBoundaryIndices.IsValidIndex(SlotB));
	bool bResult18 = false;
	bool bResult19 = false;
	if (bResult17)
	{
		const int32 First = NodeBPtr->OuterBoundaryIndices[SlotB];
		const int32 Second = NodeBPtr->OuterBoundaryIndices[(SlotB + 1) % CountB];
		
		bResult18 = TestTrue(TEXT("NodeB's portal slot should resolve to a boundary edge containing shared vertex Idx0"), First == Fixture.Idx0 || Second == Fixture.Idx0);
		bResult19 = TestTrue(TEXT("NodeB's portal slot should resolve to a boundary edge containing shared vertex Idx2"), First == Fixture.Idx2 || Second == Fixture.Idx2);
	}
	
	const FDuplicateEdgeFixture DuplicateFixture = BuildDuplicateEdgeGraph();
	TGuardValue SuppressEnsureErrors(FAutomationTestBase::bSuppressLogErrors, true);
	const int32 FoundEdgeIndex = DuplicateFixture.Graph->FindEdgeBetween(DuplicateFixture.NodeAIndex, DuplicateFixture.NodeBIndex);

	const bool bResult20 = TestEqual(TEXT("FindEdgeBetween should return the first-added edge when two portal edges connect the same node pair, per its documented first-wins contract"), FoundEdgeIndex, DuplicateFixture.FirstEdgeIndex);
	
	return bResult1 && bResult2 && bResult3 && bResult4 && bResult5 && bResult6 && bResult7 && bResult8 && bResult9
	&& bResult10 && bResult11 && bResult12 && bResult13 && bResult14 && bResult15 && bResult16 && bResult17 && bResult18 
	&& bResult19 && bResult20;
}

#endif // WITH_DEV_AUTOMATION_TESTS