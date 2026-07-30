#include "SurfaceGraphTestUtils.h"

FTwoNodeDiagonalSquareFixture SurfaceGraphTestUtils::BuildTwoNodeDiagonalSquareGraph()
{
	USurfaceGraph* SurfaceGraph = NewObject<USurfaceGraph>();
	FTwoNodeDiagonalSquareFixture Fixture;

	const int32 Idx0 = SurfaceGraph->AddVertex(FVector(0, 0, 0));
	const int32 Idx1 = SurfaceGraph->AddVertex(FVector(10, 0, 0));
	const int32 Idx2 = SurfaceGraph->AddVertex(FVector(10, 10, 0));
	const int32 Idx3 = SurfaceGraph->AddVertex(FVector(0, 10, 0));

	const int32 NodeAIndex = AddTriangleNode(*SurfaceGraph, Idx0, Idx1, Idx2, FVector(0,0,1));
	const int32 NodeBIndex = AddTriangleNode(*SurfaceGraph, Idx2, Idx3, Idx0, FVector(0,0,1));
	const int32 EdgeIndex = AddPortalEdge(*SurfaceGraph, NodeAIndex, 2, NodeBIndex, 2, false, false);

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
	
	const int32 Idx0= SurfaceGraph->AddVertex(FVector(0, 0, 0));
	const int32 Idx1 = SurfaceGraph->AddVertex(FVector(10, 0, 0));
	const int32 Idx2 = SurfaceGraph->AddVertex(FVector(10, 10, 0));
	const int32 Idx3 = SurfaceGraph->AddVertex(FVector(0, 10, 0));
	const int32 Idx4 = SurfaceGraph->AddVertex(FVector(20, 10, 0));
	const int32 Idx5 = SurfaceGraph->AddVertex(FVector(20, 20, 0));
	const int32 Idx6 = SurfaceGraph->AddVertex(FVector(10, 20, 0));
	
	const int32 NodeSIndex = AddTriangleNode(*SurfaceGraph, Idx2, Idx3, Idx0, FVector(0,0,1));
	const int32 NodeM1Index = AddTriangleNode(*SurfaceGraph, Idx0, Idx1, Idx2, FVector(0,0,1));
	const int32 NodeM2Index = AddTriangleNode(*SurfaceGraph, Idx4, Idx2, Idx1, FVector(0,0,1));
	const int32 NodeM3Index = AddTriangleNode(*SurfaceGraph, Idx2, Idx4, Idx5, FVector(0,0,1));
	const int32 NodeGIndex = AddTriangleNode(*SurfaceGraph, Idx5, Idx6, Idx2, FVector(0,0,1));

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

FMultiTriangleNodeFixture SurfaceGraphTestUtils::BuildMultiTriangleNodeGraph()
{
	USurfaceGraph* SurfaceGraph = NewObject<USurfaceGraph>();
	FMultiTriangleNodeFixture Fixture;
	
	const int32 IdxA = SurfaceGraph->AddVertex(FVector(0, 0, 0));
	const int32 IdxB = SurfaceGraph->AddVertex(FVector(24, 0, 0));
	const int32 IdxC = SurfaceGraph->AddVertex(FVector(0, 18, 0));
	const int32 IdxAB = SurfaceGraph->AddVertex(FVector(16, 0, 0));
	const int32 IdxBC = SurfaceGraph->AddVertex(FVector(12, 9, 0));
	const int32 IdxCA = SurfaceGraph->AddVertex(FVector(0, 9, 0));
	
	FSurfaceGraphNode Node;
	Node.OuterBoundaryIndices = {IdxA, IdxAB, IdxB, IdxBC, IdxC, IdxCA};
	Node.TriangleVertexIndices = { 
		IdxA,  IdxAB, IdxCA,	// T0 — ordinal 0
		IdxAB, IdxB,  IdxBC,	// T1 — ordinal 1
		IdxCA, IdxBC, IdxC,		// T2 — ordinal 2
		IdxAB, IdxBC, IdxCA		// T3 — ordinal 3 (center)
	};
	
	Node.Normal = FVector(0,0,1);
	const int32 NodeIndex = SurfaceGraph->AddNode(Node);
	
	Fixture.Graph = SurfaceGraph;
	Fixture.NodeIndex = NodeIndex;
	Fixture.CornerTriangleIndex0 = 0;
	Fixture.CornerTriangleIndex1 = 1;
	Fixture.CornerTriangleIndex2 = 2;
	Fixture.CenterTriangleIndex = 3;
	
	return Fixture;
}

FDuplicateEdgeFixture SurfaceGraphTestUtils::BuildDuplicateEdgeGraph()
{
	USurfaceGraph* SurfaceGraph = NewObject<USurfaceGraph>();
	FDuplicateEdgeFixture Fixture;
	
	const int32 Idx0 = SurfaceGraph->AddVertex(FVector(0, 0, 0));
	const int32 Idx1 = SurfaceGraph->AddVertex(FVector(10, 0, 0));
	const int32 Idx2 = SurfaceGraph->AddVertex(FVector(10, 10, 0));
	const int32 Idx3 = SurfaceGraph->AddVertex(FVector(0, 10, 0));

	const int32 NodeAIndex = AddTriangleNode(*SurfaceGraph, Idx0, Idx1, Idx2, FVector(0,0,1));
	const int32 NodeBIndex = AddTriangleNode(*SurfaceGraph, Idx2, Idx3, Idx0, FVector(0,0,1));
	const int32 FirstEdgeIndex = AddPortalEdge(*SurfaceGraph, NodeAIndex, 0, NodeBIndex, 2, false, false);
	const int32 SecondEdgeIndex = AddPortalEdge(*SurfaceGraph, NodeAIndex, 0, NodeBIndex, 2, false, false);

	Fixture.Graph = SurfaceGraph;
	Fixture.NodeAIndex = NodeAIndex;
	Fixture.NodeBIndex = NodeBIndex;
	Fixture.FirstEdgeIndex = FirstEdgeIndex;
	Fixture.SecondEdgeIndex = SecondEdgeIndex;
	
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

int32 SurfaceGraphTestUtils::AddEmptyNode(USurfaceGraph& Graph)
{
	FSurfaceGraphNode Node;
	Node.Normal = FVector(0,0,1);
	return Graph.AddNode(Node);
}
