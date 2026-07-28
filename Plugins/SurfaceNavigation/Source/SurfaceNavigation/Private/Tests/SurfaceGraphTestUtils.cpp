#include "SurfaceGraphTestUtils.h"

FTwoNodeDiagonalSquareFixture SurfaceGraphTestUtils::BuildTwoNodeDiagonalSquareGraph()
{
	USurfaceGraph* SurfaceGraph = NewObject<USurfaceGraph>();
	FTwoNodeDiagonalSquareFixture Fixture;
	
	int32 Idx0 = SurfaceGraph->AddVertex(FVector(0, 0, 0));
	int32 Idx1 = SurfaceGraph->AddVertex(FVector(10, 0, 0));
	int32 Idx2 = SurfaceGraph->AddVertex(FVector(10, 10, 0));
	int32 Idx3 = SurfaceGraph->AddVertex(FVector(0, 10, 0));

	const int32 NodeAIndex = SurfaceGraphTestUtils::AddTriangleNode(*SurfaceGraph, Idx0, Idx1, Idx2, FVector(0,0,1));
	const int32 NodeBIndex = SurfaceGraphTestUtils::AddTriangleNode(*SurfaceGraph, Idx2, Idx3, Idx0, FVector(0,0,1));
	const int32 EdgeIndex = SurfaceGraphTestUtils::AddPortalEdge(*SurfaceGraph, NodeAIndex, 2, NodeBIndex, 2, false, false);

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

FDecoyCycleGraphFixture SurfaceGraphTestUtils::BuildDecoyCycleGraph()
{
	USurfaceGraph* SurfaceGraph = NewObject<USurfaceGraph>();
	FDecoyCycleGraphFixture Fixture;
	
	const int32 IdxA = SurfaceGraph->AddVertex(FVector(0, 0, 0));
	const int32 IdxB = SurfaceGraph->AddVertex(FVector(10, 0, 0));
	const int32 IdxC = SurfaceGraph->AddVertex(FVector(10, 10, 0));
	const int32 IdxD = SurfaceGraph->AddVertex(FVector(0, 10, 0));
	const int32 IdxE = SurfaceGraph->AddVertex(FVector(20, 10, 0));
	const int32 IdxF = SurfaceGraph->AddVertex(FVector(20, 20, 0));
	const int32 IdxH = SurfaceGraph->AddVertex(FVector(10, 20, 0));
	
	const int32 NodeSIndex = SurfaceGraphTestUtils::AddTriangleNode(*SurfaceGraph, IdxC, IdxD, IdxA, FVector(0,0,1));
	const int32 NodeM1Index = SurfaceGraphTestUtils::AddTriangleNode(*SurfaceGraph, IdxA, IdxB, IdxC, FVector(0,0,1));
	const int32 NodeM2Index = SurfaceGraphTestUtils::AddTriangleNode(*SurfaceGraph, IdxE, IdxC, IdxB, FVector(0,0,1));
	const int32 NodeM3Index = SurfaceGraphTestUtils::AddTriangleNode(*SurfaceGraph, IdxC, IdxE, IdxF, FVector(0,0,1));
	const int32 NodeGIndex = SurfaceGraphTestUtils::AddTriangleNode(*SurfaceGraph, IdxF, IdxH, IdxC, FVector(0,0,1));

	AddPortalEdge(*SurfaceGraph, NodeSIndex, 2, NodeM1Index, 2, false, false);
	AddPortalEdge(*SurfaceGraph, NodeM1Index, 1, NodeM2Index, 1, false, false);
	AddPortalEdge(*SurfaceGraph, NodeM2Index, 0, NodeM3Index, 0, false, false);
	AddPortalEdge(*SurfaceGraph, NodeM3Index, 2, NodeGIndex, 2, false, false);
	AddPortalEdge(*SurfaceGraph, NodeSIndex, 0, NodeGIndex, 1, true, false);

	Fixture.Graph = SurfaceGraph;
	Fixture.NodeSIndex = NodeSIndex;
	Fixture.NodeM1Index = NodeM1Index;
	Fixture.NodeM2Index = NodeM2Index;
	Fixture.NodeM3Index = NodeM3Index;
	Fixture.NodeGIndex = NodeGIndex;
	
	return Fixture;
}

int32 SurfaceGraphTestUtils::AddIsolatedNode(USurfaceGraph& Graph)
{
	const int32 IdxI1 = Graph.AddVertex(FVector(100, 100, 0));
	const int32 IdxI2 = Graph.AddVertex(FVector(110, 100, 0));
	const int32 IdxI3 = Graph.AddVertex(FVector(100, 110, 0));
	
	FSurfaceGraphNode Node;
	Node.OuterBoundaryIndices = {IdxI1, IdxI2, IdxI3};
	Node.TriangleVertexIndices ={IdxI1, IdxI2, IdxI3};
	Node.Normal = FVector(0,0,1);
		
	return Graph.AddNode(Node);
}

int32 SurfaceGraphTestUtils::AddTriangleNode(USurfaceGraph& Graph, const int32 V0, const int32 V1, const int32 V2, const FVector& Normal)
{
	FSurfaceGraphNode Node;
	Node.OuterBoundaryIndices = {V0, V1, V2};
	Node.TriangleVertexIndices = {V0, V1, V2};
	Node.Normal = Normal;

	return Graph.AddNode(Node);
}

int32 SurfaceGraphTestUtils::AddPortalEdge(USurfaceGraph& Graph, const int32 NodeAIndex, const int32 SlotA, const int32 NodeBIndex, const int32 SlotB, const bool bIsGapBridge, const bool bRequiresReorientation)
{
	FSurfaceGraphEdge Edge;
	Edge.NodeAIndex = NodeAIndex;
	Edge.NodeBIndex = NodeBIndex;
	Edge.PortalSlotA = SlotA;
	Edge.PortalSlotB = SlotB;
	Edge.bRequiresReorientation = bRequiresReorientation;
	Edge.bIsGapBridge = bIsGapBridge;
	const int32 EdgeIndex = Graph.AddEdge(Edge);
	Graph.AddNeighborEdgeToNode(NodeAIndex, EdgeIndex);
	Graph.AddNeighborEdgeToNode(NodeBIndex, EdgeIndex);

	return EdgeIndex;
}
