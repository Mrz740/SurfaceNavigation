#include "Misc/AutomationTest.h"
#include "SurfaceGraph.h"

#if WITH_DEV_AUTOMATION_TESTS

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FSurfaceGraphTests, "SurfaceNavigation.SurfaceGraph.TwoNodePortalGraph",
                                 EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)

bool FSurfaceGraphTests::RunTest(const FString& Parameters)
{
	USurfaceGraph* SurfaceGraph = NewObject<USurfaceGraph>();

	const FVector V0 = FVector(0, 0, 0);
	const FVector V1 = FVector(10, 0, 0);
	const FVector V2 = FVector(10, 10, 0);
	const FVector V3 = FVector(0, 10, 0);
	
	int32 Idx0 = SurfaceGraph->AddVertex(V0);
	int32 Idx1 = SurfaceGraph->AddVertex(V1);
	int32 Idx2 = SurfaceGraph->AddVertex(V2);
	int32 Idx3 = SurfaceGraph->AddVertex(V3);

	FSurfaceGraphNode NodeA;
	NodeA.OuterBoundaryIndices = {Idx0, Idx1, Idx2};
	NodeA.TriangleVertexIndices = {Idx0,Idx1,Idx2};
	NodeA.Normal = FVector(0,0,1);
	int32 NodeAIndex = SurfaceGraph->AddNode(NodeA);
	
	FSurfaceGraphNode NodeB;
	NodeB.OuterBoundaryIndices = {Idx2, Idx3, Idx0};
	NodeB.TriangleVertexIndices = {Idx2,Idx3,Idx0};
	NodeB.Normal = FVector(0,0,1);
	int32 NodeBIndex = SurfaceGraph->AddNode(NodeB);
	
	FSurfaceGraphEdge Edge;
	Edge.NodeAIndex = NodeAIndex;
	Edge.NodeBIndex = NodeBIndex;
	int32 EdgeIndex = SurfaceGraph->AddEdge(Edge);
	
	SurfaceGraph->AddNeighborEdgeToNode(NodeAIndex, EdgeIndex);
	SurfaceGraph->AddNeighborEdgeToNode(NodeBIndex, EdgeIndex);
	
	const FSurfaceGraphNode* NodeAPtr = SurfaceGraph->GetNode(NodeAIndex);
	const FSurfaceGraphNode* NodeBPtr = SurfaceGraph->GetNode(NodeBIndex);
	const FSurfaceGraphEdge* EdgePtr = SurfaceGraph->GetEdge(EdgeIndex);
	
	bool bResult1 = TestEqual(TEXT("Edge should reference NodeA by index"), EdgePtr->NodeAIndex,NodeAIndex);
	bool bResult2 = TestEqual(TEXT("Edge should reference NodeB by index"), EdgePtr->NodeBIndex,NodeBIndex);
	bool bResult3 = TestTrue(TEXT("NodeA's NeighbourEdgeIndices should contain the shared edge"), NodeAPtr->NeighborEdgeIndices.Contains(EdgeIndex));
	bool bResult4 = TestTrue(TEXT("NodeB's NeighbourEdgeIndices should contain the shared edge"), NodeBPtr->NeighborEdgeIndices.Contains(EdgeIndex));
	bool bResult5 = TestEqual(TEXT("Shared vertex pool should hold exactly 4 entries, not 8"),SurfaceGraph->GetSharedVertexCount(),4);
	bool bResult6 = TestTrue(TEXT("NodeA's boundary should reference the shared corner index"),NodeAPtr->OuterBoundaryIndices.Contains(Idx0));
	bool bResult7 = TestTrue(TEXT("NodeB's boundary should reference the shared corner index"),NodeBPtr->OuterBoundaryIndices.Contains(Idx0));
	
	const FVector InsidePoint = FVector(7,2,0);
	const FVector OutsidePoint = FVector(2,7,0);
	const FVector BoundaryPoint = FVector(5,5,0);
	
	bool bResult8 = TestTrue(TEXT("Point strictly inside NodeA should return true"),SurfaceGraph->IsPointInPolygon(*NodeAPtr,InsidePoint));
	bool bResult9 = TestFalse(TEXT("Point strictly outside NodeA should return false"),SurfaceGraph->IsPointInPolygon(*NodeAPtr,OutsidePoint));
	bool bResult10 = TestTrue(TEXT("Point exactly on the shared boundary edge should return true"),SurfaceGraph->IsPointInPolygon(*NodeAPtr,BoundaryPoint));
	
	const FSurfaceGraphNode* InvalidNodePtr = SurfaceGraph->GetNode(999);
	const FSurfaceGraphEdge* InvalidEdgePtr = SurfaceGraph->GetEdge(999);
	const FSurfaceGraphNode* NegativeNodePtr = SurfaceGraph->GetNode(-1);

	bool bResult11 = TestNull(TEXT("Out-of-range node index should return nullptr, not crash"),InvalidNodePtr);
	bool bResult12 = TestNull(TEXT("Out-of-range edge index should return nullptr, not crash"),InvalidEdgePtr);
	bool bResult13 = TestNull(TEXT("Negative node index should return nullptr, not crash"),NegativeNodePtr);

	return bResult1 && bResult2 && bResult3 && bResult4 && bResult5 && bResult6 && bResult7 && bResult8 && bResult9 && bResult10 && bResult11 && bResult12 && bResult13;
}

#endif // WITH_DEV_AUTOMATION_TESTS