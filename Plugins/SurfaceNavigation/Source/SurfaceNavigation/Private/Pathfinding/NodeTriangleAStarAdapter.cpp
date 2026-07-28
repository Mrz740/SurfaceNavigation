#include "Pathfinding/NodeTriangleAStarAdapter.h"

FNodeTriangleAStarAdapter::FNodeTriangleAStarAdapter(const USurfaceGraph& InGraph, const int32 InNodeIndex) : SurfaceGraph(InGraph), NodeIndex(InNodeIndex)
{
	const FSurfaceGraphNode* NodePtr = SurfaceGraph.GetNode(InNodeIndex);
	if (!NodePtr)
	{
		ensureMsgf(false, TEXT(""));
		return;
	}
	
	const TArray<int32>& Indices = NodePtr->TriangleVertexIndices;
	ensureMsgf(Indices.Num() % 3 == 0, TEXT(""));
	const int32 TriangleCount = Indices.Num() / 3;
	
	TriangleCenters.Reserve(TriangleCount);
	TriangleLinks.SetNum(3 * TriangleCount);
	
	TMap<TPair<int32,int32>,int32> EdgeToSlot;
	EdgeToSlot.Reserve(3 * TriangleCount);
	
	for (int32 t = 0; t < TriangleCount; t++)
	{
		const FVector* P0 = SurfaceGraph.GetVertex(Indices[3 * t]);
		const FVector* P1 = SurfaceGraph.GetVertex(Indices[3 * t + 1]);
		const FVector* P2 = SurfaceGraph.GetVertex(Indices[3 * t + 2]);
		TriangleCenters.Add((*P0 + *P1 + *P2) / 3.0);

		for (int32 j = 0; j < 3; j++)
		{
			const int32 A = Indices[3 * t + j];
			const int32 B = Indices[3 * t + (j + 1) % 3];
			TPair<int32, int32> Key = {FMath::Min(A, B), FMath::Max(A, B)};
			int32 Slot = 3 * t + j;
			
			const int32* Existing = EdgeToSlot.Find(Key);
			
			if (!Existing)
			{
				EdgeToSlot.Add(Key, Slot);
				continue;
			}
			
			if (TriangleLinks[*Existing].NeighborOrdinal != INDEX_NONE)
			{
				ensureMsgf(false, TEXT(""));
				continue;
			}
			
			FVector MidPoint = (*SurfaceGraph.GetVertex(A) + *SurfaceGraph.GetVertex(B)) * 0.5;
			
			TriangleLinks[Slot].NeighborOrdinal = *Existing / 3;
			TriangleLinks[Slot].EdgeMidpoint = MidPoint;
			TriangleLinks[*Existing].NeighborOrdinal = t;
			TriangleLinks[*Existing].EdgeMidpoint = MidPoint;
		}
	}
}

void FNodeTriangleAStarAdapter::GetNeighbors(int32 NodeID, TArray<int32>& OutNeighbors) const
{
	OutNeighbors.Reset();
	// TODO Implement this function
}

float FNodeTriangleAStarAdapter::GetCost(int32 FromNodeID, int32 ToNodeID) const
{
	// TODO Implement this function
	return 0.0f;
}

float FNodeTriangleAStarAdapter::GetHeuristic(int32 FromNodeID, int32 GoalNodeID) const
{
	// TODO Implement this function
	return 0.0f;
}
