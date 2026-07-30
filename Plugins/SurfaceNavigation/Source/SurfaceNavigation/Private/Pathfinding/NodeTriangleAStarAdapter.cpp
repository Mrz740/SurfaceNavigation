#include "Pathfinding/NodeTriangleAStarAdapter.h"

FNodeTriangleAStarAdapter::FNodeTriangleAStarAdapter(const USurfaceGraph& InGraph, const int32 InNodeIndex) : SurfaceGraph(InGraph), NodeIndex(InNodeIndex)
{
	const FSurfaceGraphNode* NodePtr = SurfaceGraph.GetNode(InNodeIndex);
	if (!NodePtr)
	{
		ensureMsgf(false, TEXT("Node %d is not a valid index; adapter will report no triangles"), InNodeIndex);
		return;
	}
	
	USurfaceGraph::BuildPlaneBasis(NodePtr->Normal, CachedTangent1, CachedTangent2);
	
	const TArray<int32>& Indices = NodePtr->TriangleVertexIndices;
	ensureMsgf(Indices.Num() % 3 == 0, TEXT("Node %d has %d triangle vertex indices, which is not a multiple of 3; the trailing partial triangle will be ignored"), InNodeIndex, Indices.Num());
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
				ensureMsgf(false, TEXT("Non-manifold geometry in node %d: edge (%d, %d) is claimed by three or more triangles; treating triangle %d's edge %d as a border"), InNodeIndex, A, B, t, j);
				continue;
			}

			const FVector MidPoint = (*SurfaceGraph.GetVertex(A) + *SurfaceGraph.GetVertex(B)) * 0.5;
			
			TriangleLinks[Slot].NeighborOrdinal = *Existing / 3;
			TriangleLinks[Slot].EdgeMidpoint = MidPoint;
			TriangleLinks[*Existing].NeighborOrdinal = t;
			TriangleLinks[*Existing].EdgeMidpoint = MidPoint;
		}
	}
}

void FNodeTriangleAStarAdapter::GetNeighbors(const int32 TriangleID, TArray<int32>& OutNeighbors) const
{
	OutNeighbors.Reset();
	
	if (!TriangleCenters.IsValidIndex(TriangleID)) return;
	
	for (int32 i = 0 ; i < 3; i++)
	{
		if (const int32 Ordinal = TriangleLinks[3 * TriangleID + i].NeighborOrdinal; Ordinal != INDEX_NONE)
		{
			OutNeighbors.Add(Ordinal);
		}
	}
}

float FNodeTriangleAStarAdapter::GetCost(const int32 FromTriangleID, const int32 ToTriangleID) const
{
	if (!TriangleCenters.IsValidIndex(FromTriangleID))
	{
		checkf(false, TEXT("Triangle %d is not a valid index in node %d (TriangleCount=%d)"), FromTriangleID, NodeIndex, TriangleCenters.Num());
		return 0.0f;
	}
	
	for (int32 i = 0 ; i < 3; i++)
	{
		const FTriangleEdgeLink& Link = TriangleLinks[3 * FromTriangleID + i];
		if (Link.NeighborOrdinal == INDEX_NONE) continue; 
		if (Link.NeighborOrdinal == ToTriangleID)
		{
			const FVector M = Link.EdgeMidpoint;
			return FVector::Dist(TriangleCenters[FromTriangleID],M) + FVector::Dist(M,TriangleCenters[ToTriangleID]);
		}
	}
	
	checkf(false, TEXT("No shared edge between triangles %d and %d in node %d"), FromTriangleID, ToTriangleID, NodeIndex);

	return 0.0f;
}

float FNodeTriangleAStarAdapter::GetHeuristic(const int32 FromTriangleID, const int32 GoalTriangleID) const
{
	if (!TriangleCenters.IsValidIndex(FromTriangleID) || !TriangleCenters.IsValidIndex(GoalTriangleID))
	{
		return 0.0f;
	}
	
	return FVector::Dist(TriangleCenters[FromTriangleID], TriangleCenters[GoalTriangleID]);
}

bool FNodeTriangleAStarAdapter::FindNearestTriangle(const FVector& Point, int32& OutTriangleID) const
{
	OutTriangleID = INDEX_NONE;
	const FSurfaceGraphNode* NodePtr = SurfaceGraph.GetNode(NodeIndex);
	
	if (TriangleCenters.IsEmpty()) return false;
	
	checkf(NodePtr != nullptr, TEXT("Node %d is not a valid index"), NodeIndex);

	for (int32 t = 0; t < TriangleCenters.Num(); t++)
	{
		TArrayView TriVerts(NodePtr->TriangleVertexIndices.GetData() + 3 * t, 3);
		
		if (SurfaceGraph.TestPointAgainstLoop(TriVerts, Point, CachedTangent1, CachedTangent2) != ELoopContainment::Outside)
		{
			OutTriangleID = t;
			return true;
		}
	}
	
	int32 BestTriangle = 0;
	double BestDistSquared = FVector::DistSquared(Point, TriangleCenters[0]);

	for (int32 t = 1; t < TriangleCenters.Num(); t++)
	{
		if (const double D = FVector::DistSquared(Point, TriangleCenters[t]); D < BestDistSquared)
		{
			BestDistSquared = D;
			BestTriangle    = t;
		}
	}
	
	OutTriangleID = BestTriangle;
	return true;
}
