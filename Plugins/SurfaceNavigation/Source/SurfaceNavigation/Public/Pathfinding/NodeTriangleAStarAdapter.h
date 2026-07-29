#pragma once

#include "CoreMinimal.h"
#include "AStarGraphAdapter.h"
#include "SurfaceGraph.h"


class FNodeTriangleAStarAdapter final : public IAStarGraphAdapter
{
public:
	~FNodeTriangleAStarAdapter() = default;
	explicit FNodeTriangleAStarAdapter(const USurfaceGraph& InGraph, const int32 InNodeIndex);	
	
	virtual void GetNeighbors(const int32 TriangleID, TArray<int32>& OutNeighbors) const override;
	virtual float GetCost(const int32 FromTriangleID, const int32 ToTriangleID) const override;
	virtual float GetHeuristic(const int32 FromTriangleID, const int32 GoalTriangleID) const override;
	
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
