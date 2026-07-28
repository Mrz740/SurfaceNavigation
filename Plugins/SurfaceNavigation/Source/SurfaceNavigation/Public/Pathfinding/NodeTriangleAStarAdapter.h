#pragma once

#include "CoreMinimal.h"
#include "AStarGraphAdapter.h"
#include "SurfaceGraph.h"


class FNodeTriangleAStarAdapter final : public IAStarGraphAdapter
{
public:
	~FNodeTriangleAStarAdapter() = default;
	explicit FNodeTriangleAStarAdapter(const USurfaceGraph& InGraph, const int32 InNodeIndex);	
	
	virtual void GetNeighbors(int32 NodeID, TArray<int32>& OutNeighbors) const override;
	virtual float GetCost(int32 FromNodeID, int32 ToNodeID) const override;
	virtual float GetHeuristic(int32 FromNodeID, int32 GoalNodeID) const override;
	
private:
	struct FTriangleEdgeLink
	{
		int32 NeighborOrdinal = INDEX_NONE;
		FVector EdgeMidpoint = FVector::ZeroVector;
	};

	const USurfaceGraph& SurfaceGraph;
	int32 NodeIndex = INDEX_NONE;
	TArray<FVector> TriangleCenters;
	TArray<FTriangleEdgeLink> TriangleLinks;
};
