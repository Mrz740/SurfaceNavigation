#include "Pathfinding/AStarPathFinder.h"

namespace 
{
	struct FOpenSetEntry
	{
		int32 NodeID = INDEX_NONE;
		float FCost = 0.0f;
	};
	
	struct FSearchNodeState
	{
		float GCost = 0.0f;
		int32 Parent = INDEX_NONE;
		bool bClosed = false;
	};
}

bool FindAStarPath(const IAStarGraphAdapter& Adapter, const int32 StartNodeID, const int32 GoalNodeID,
	TArray<int32>& OutPath)
{
	OutPath.Reset();
	
	TArray<FOpenSetEntry> OpenSet;
	TMap<int32, FSearchNodeState> NodeStates;
	TArray<int32> Neighbors;

	bool bSuccess = false;
	
	auto FCostPredicate = [](const FOpenSetEntry& A, const FOpenSetEntry& B){ return A.FCost < B.FCost; };
	
	OpenSet.HeapPush({.NodeID = StartNodeID, .FCost = Adapter.GetHeuristic(StartNodeID, GoalNodeID)},FCostPredicate);
	NodeStates.Add(StartNodeID, {0.0f,INDEX_NONE,false});

	while (!OpenSet.IsEmpty())
	{
		FOpenSetEntry Popped;
		OpenSet.HeapPop(Popped, FCostPredicate);
		
		if (NodeStates.Find(Popped.NodeID)->bClosed) continue;
		
		if (Popped.NodeID == GoalNodeID)
		{
			int32 CurrentID = Popped.NodeID;
			while (CurrentID != INDEX_NONE)
			{
				OutPath.Add(CurrentID);
				CurrentID = NodeStates[CurrentID].Parent;
			}
			Algo::Reverse(OutPath);
			bSuccess = true;
			break;
		}
		
		NodeStates[Popped.NodeID].bClosed = true;
		
		Adapter.GetNeighbors(Popped.NodeID, Neighbors);
		
		for (int32 NeighborID : Neighbors)
		{
			const FSearchNodeState* NeighborState = NodeStates.Find(NeighborID);
			if (NeighborState != nullptr && NeighborState->bClosed) continue;
			
			const float NewGCost = NodeStates[Popped.NodeID].GCost + Adapter.GetCost(Popped.NodeID,NeighborID);
			
			if (NeighborState == nullptr || NewGCost < NeighborState->GCost)
			{
				NodeStates.Add(NeighborID, FSearchNodeState(NewGCost, Popped.NodeID, false));
				OpenSet.HeapPush({NeighborID, NewGCost + Adapter.GetHeuristic(NeighborID, GoalNodeID)}, FCostPredicate);
			}
		}
	}
	
	return bSuccess;
}
