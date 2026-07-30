#include "Pathfinding/SurfaceGraphAStarAdapter.h"

FSurfaceGraphAStarAdapter::FSurfaceGraphAStarAdapter(const USurfaceGraph& InGraph) : SurfaceGraph(InGraph)
{
	NodeCenters.Reserve(SurfaceGraph.GetNodeCount());
	for (int32 i = 0; i < SurfaceGraph.GetNodeCount(); i++)
	{
		const FSurfaceGraphNode* NodePtr = SurfaceGraph.GetNode(i); 
		FVector Sum = FVector::ZeroVector;
		for (const int32 VertexIndex : NodePtr->OuterBoundaryIndices)
		{
			Sum += *SurfaceGraph.GetVertex(VertexIndex);
		}
		NodeCenters.Add(Sum / NodePtr->OuterBoundaryIndices.Num());
	}
	
	PortalMidpointsA.Reserve(SurfaceGraph.GetEdgeCount());
	PortalMidpointsB.Reserve(SurfaceGraph.GetEdgeCount());

	for (int32 i = 0; i < SurfaceGraph.GetEdgeCount(); i++)
	{	
		const FSurfaceGraphEdge* EdgePtr = SurfaceGraph.GetEdge(i);
		PortalMidpointsA.Add(SurfaceGraph.GetPortalMidpoint(EdgePtr->NodeAIndex, EdgePtr->PortalSlotA));
		PortalMidpointsB.Add(SurfaceGraph.GetPortalMidpoint(EdgePtr->NodeBIndex, EdgePtr->PortalSlotB));
	}
}

void FSurfaceGraphAStarAdapter::GetNeighbors(const int32 NodeID, TArray<int32>& OutNeighbors) const
{
	OutNeighbors.Reset();
	const FSurfaceGraphNode* NodePtr = SurfaceGraph.GetNode(NodeID);
	
	if (NodePtr == nullptr) return;
	
	for (const int32 EdgeIndex : NodePtr->NeighborEdgeIndices)
	{
		const FSurfaceGraphEdge* EdgePtr = SurfaceGraph.GetEdge(EdgeIndex);
		if (EdgePtr == nullptr) continue;
		if (EdgePtr->NodeAIndex == NodeID) OutNeighbors.Add(EdgePtr->NodeBIndex);
		else OutNeighbors.Add(EdgePtr->NodeAIndex);
	}
}

float FSurfaceGraphAStarAdapter::GetCost(const int32 FromNodeID, const int32 ToNodeID) const
{
	const int32 EdgeIndex = SurfaceGraph.FindEdgeBetween(FromNodeID, ToNodeID);
	
	if (EdgeIndex != INDEX_NONE)
	{
		return FVector::Dist(NodeCenters[FromNodeID], PortalMidpointsA[EdgeIndex]) +
			   FVector::Dist(PortalMidpointsA[EdgeIndex], PortalMidpointsB[EdgeIndex]) +
			   FVector::Dist(PortalMidpointsB[EdgeIndex], NodeCenters[ToNodeID]);
	}

	checkf(false, TEXT("No edge found between nodes %d and %d"), FromNodeID, ToNodeID);
	
	return 0.0f;
}

float FSurfaceGraphAStarAdapter::GetHeuristic(const int32 FromNodeID, const int32 GoalNodeID) const
{
	if (SurfaceGraph.GetNode(FromNodeID) == nullptr || SurfaceGraph.GetNode(GoalNodeID) == nullptr)
	{
		return 0.0f;
	}

	return FVector::Dist(NodeCenters[FromNodeID],NodeCenters[GoalNodeID]);
}
