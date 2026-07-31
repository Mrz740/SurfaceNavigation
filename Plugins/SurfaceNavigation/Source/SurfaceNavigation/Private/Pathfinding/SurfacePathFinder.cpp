#include "Pathfinding/SurfacePathFinder.h"
#include "Pathfinding/AStarPathFinder.h"
#include "Pathfinding/NodeTriangleAStarAdapter.h"
#include "Pathfinding/SurfaceGraphAStarAdapter.h"

bool FindSurfacePath(const USurfaceGraph& Graph, const FVector& StartPosition, int32 StartNodeIndex,
                     const FVector& GoalPosition, int32 GoalNodeIndex, FSurfacePath& OutPath)
{
	OutPath.Waypoints.Reset();
	
	const FSurfaceGraphAStarAdapter MacroAdapter(Graph);

	TArray<int32> NodeSequence;
	TArray<int32> TriSequence;

	if (!FindAStarPath(MacroAdapter, StartNodeIndex, GoalNodeIndex,NodeSequence)) return false;

	FVector EntryPoint = StartPosition;
	FVector ExitPoint;

	TOptional<FSurfaceTransitionInfo> PendingTransition;
	
	for (int32 i = 0; i < NodeSequence.Num(); i++)
	{
		const int32 NodeIdx = NodeSequence[i];
		const bool bIsLast = (i == NodeSequence.Num() - 1);
		
		const FSurfaceGraphEdge* EdgePtr = nullptr; 
		
		if (bIsLast)
		{
			ExitPoint = GoalPosition;
		} 
		
		else
		{
			const int32 Edge = Graph.FindEdgeBetween(NodeIdx, NodeSequence[i + 1]);
			checkf(Edge != INDEX_NONE, TEXT("FindEdgeBetween found no edge between nodes %d and %d, but the macro A* path already proved them adjacent"), NodeIdx, NodeSequence[i + 1]);
			EdgePtr = Graph.GetEdge(Edge);
			
			const int32 Slot = (EdgePtr->NodeAIndex == NodeIdx) ? EdgePtr->PortalSlotA : EdgePtr->PortalSlotB;
			// TODO: interim crossing point. The funnel algorithm replaces this midpoint with the tautened point along the portal, in its own follow-up milestone.
			ExitPoint = Graph.GetPortalMidpoint(NodeIdx, Slot);
		}
		
		const FNodeTriangleAStarAdapter LocalAdapter(Graph, NodeIdx);
		
		int32 EntryTri;
		int32 ExitTri;
		
		const bool bFoundEntry = LocalAdapter.FindNearestTriangle(EntryPoint, EntryTri);
		const bool bFoundExit  = LocalAdapter.FindNearestTriangle(ExitPoint, ExitTri);
		
		if (!bFoundEntry || !bFoundExit)
		{
			ensureMsgf(false, TEXT("Node %d has no triangles to search (entry or exit point could not be located)"), NodeIdx);
			OutPath.Waypoints.Reset();
			return false;
		}

		if (!FindAStarPath(LocalAdapter, EntryTri, ExitTri, TriSequence))
		{
			OutPath.Waypoints.Reset();
			return false;
		}
		
		FSurfaceWaypoint EntryWaypoint;
		EntryWaypoint.Position = EntryPoint;
		EntryWaypoint.NodeIndex = NodeIdx;
		EntryWaypoint.TransitionInfo = PendingTransition;
		OutPath.Waypoints.Add(EntryWaypoint);
		
		PendingTransition.Reset();
		
		for (int32 j = 0; j < TriSequence.Num() - 1; j++)
		{
			const int32 a = TriSequence[j];
			const int32 b = TriSequence[j + 1];
			
			FSurfaceWaypoint MidWaypoint;
			// TODO: interim crossing point, same as the portal midpoint above. The funnel algorithm replaces this shared-edge midpoint with the tautened point along that edge.
			MidWaypoint.Position = LocalAdapter.GetSharedEdgeMidpoint(a, b);
			MidWaypoint.NodeIndex = NodeIdx;
			OutPath.Waypoints.Add(MidWaypoint);
		}

		FSurfaceWaypoint ExitWaypoint;
		ExitWaypoint.Position = ExitPoint;
		ExitWaypoint.NodeIndex = NodeIdx;
		OutPath.Waypoints.Add(ExitWaypoint);
		
		if (!bIsLast)
		{
			const int32 NextNodeIdx = NodeSequence[i + 1];

			if (EdgePtr->bRequiresReorientation || EdgePtr->bIsGapBridge)
			{
				FSurfaceTransitionInfo Transition;
				Transition.DepartureNormal = Graph.GetNode(NodeIdx)->Normal;
				Transition.ArrivalNormal = Graph.GetNode(NextNodeIdx)->Normal;
				Transition.bRequiresReorientation = EdgePtr->bRequiresReorientation;
				Transition.bIsGapBridge = EdgePtr->bIsGapBridge;
				PendingTransition = Transition;
			}
			
			const int32 NextSlot = (EdgePtr->NodeAIndex == NextNodeIdx) ? EdgePtr->PortalSlotA : EdgePtr->PortalSlotB;
			EntryPoint = Graph.GetPortalMidpoint(NextNodeIdx, NextSlot);
		}
	}
	return true;
}
