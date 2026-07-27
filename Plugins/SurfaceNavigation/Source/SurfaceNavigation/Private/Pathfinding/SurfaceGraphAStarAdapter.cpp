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
		PortalMidpointsA.Add(ResolvePortalMidpoint(EdgePtr->NodeAIndex, EdgePtr->PortalSlotA));
		PortalMidpointsB.Add(ResolvePortalMidpoint(EdgePtr->NodeBIndex, EdgePtr->PortalSlotB));
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
	const FSurfaceGraphNode* NodePtr = SurfaceGraph.GetNode(FromNodeID);
	
	checkf(NodePtr != nullptr, TEXT("Node %d is not a valid index"), FromNodeID);
	
	for (const int32 EdgeIndex : NodePtr->NeighborEdgeIndices)
	{
		const FSurfaceGraphEdge* EdgePtr = SurfaceGraph.GetEdge(EdgeIndex);
		if (EdgePtr == nullptr) continue;
		if (EdgePtr->NodeAIndex == FromNodeID && EdgePtr->NodeBIndex == ToNodeID)
		{
			const FVector FromMidPoint = PortalMidpointsA[EdgeIndex];
			const FVector ToMidPoint = PortalMidpointsB[EdgeIndex];
			return FVector::Dist(NodeCenters[FromNodeID],FromMidPoint) 
				+ FVector::Dist(FromMidPoint,ToMidPoint) 
				+ FVector::Dist(ToMidPoint,NodeCenters[ToNodeID]);
		}
		if (EdgePtr->NodeBIndex == FromNodeID && EdgePtr->NodeAIndex == ToNodeID)
		{
			const FVector FromMidPoint = PortalMidpointsB[EdgeIndex];
			const FVector ToMidPoint = PortalMidpointsA[EdgeIndex];
			return FVector::Dist(NodeCenters[FromNodeID],FromMidPoint) 
				+ FVector::Dist(FromMidPoint,ToMidPoint) 
				+ FVector::Dist(ToMidPoint,NodeCenters[ToNodeID]);
		}
	}
	
	checkf(false, TEXT("No edge found between nodes %d and %d"), FromNodeID, ToNodeID);
	
	return 0.0f;
}

float FSurfaceGraphAStarAdapter::GetHeuristic(const int32 FromNodeID, const int32 GoalNodeID) const
{
	return FVector::Dist(NodeCenters[FromNodeID],NodeCenters[GoalNodeID]);
}

FVector FSurfaceGraphAStarAdapter::ResolvePortalMidpoint(const int32 NodeIndex, const int32 Slot) const
{
	const FSurfaceGraphNode* NodePtr = SurfaceGraph.GetNode(NodeIndex);
	
	checkf(NodePtr != nullptr, TEXT("Node %d is not a valid index"), NodeIndex);
	
	const int32 Count = NodePtr->OuterBoundaryIndices.Num();
	
	checkf(NodePtr->OuterBoundaryIndices.IsValidIndex(Slot), TEXT("Portal slot %d is not a valid index into node %d's OuterBoundaryIndices (Num=%d)"), Slot, NodeIndex, Count);
	
	const FVector First = *SurfaceGraph.GetVertex(NodePtr->OuterBoundaryIndices[Slot]);
	const FVector Second = *SurfaceGraph.GetVertex(NodePtr->OuterBoundaryIndices[(Slot + 1) % Count]);
	
	return (First + Second) * 0.5f;
}
