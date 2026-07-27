#include "SurfaceGraphTestUtils.h"

FTwoNodeDiagonalSquareFixture SurfaceGraphTestUtils::BuildTwoNodeDiagonalSquareGraph()
{
	USurfaceGraph* SurfaceGraph = NewObject<USurfaceGraph>();
	FTwoNodeDiagonalSquareFixture Fixture;
	
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
	Edge.PortalSlotA = 2;
	Edge.PortalSlotB = 2;
	Edge.bRequiresReorientation = false;
	Edge.bIsGapBridge = false;
	int32 EdgeIndex = SurfaceGraph->AddEdge(Edge);
	
	SurfaceGraph->AddNeighborEdgeToNode(NodeAIndex, EdgeIndex);
	SurfaceGraph->AddNeighborEdgeToNode(NodeBIndex, EdgeIndex);
	
	Fixture.Graph = SurfaceGraph;
	Fixture.NodeAIndex = NodeAIndex;
	Fixture.NodeBIndex = NodeBIndex;
	Fixture.EdgeIndex = EdgeIndex;
	Fixture.Idx0 = Idx0;
	Fixture.Idx1 = Idx1;
	Fixture.Idx2 = Idx2;
	Fixture.Idx3 = Idx3;
	return Fixture;
}
