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
	
	bool FindNearestTriangle(const FVector& Point, int32& OutTriangleID) const;

	/**
	 * Returns the midpoint of the edge two adjacent triangles share.
	 *
	 * This is the local tier's equivalent of a portal midpoint: the point at which a path crossing from
	 * one triangle into its neighbor passes between them. The value is precomputed during construction
	 * and read back from the same adjacency slot GetCost uses, so no geometry is recalculated here.
	 *
	 * Both arguments must name triangles that are genuinely adjacent. Callers walking a path produced by
	 * FindAStarPath over this adapter satisfy that by construction, since consecutive entries in such a
	 * path came from GetNeighbors and therefore share an edge.
	 *
	 * @param FromTriangleID  The triangle being left. Must be a valid local triangle ordinal.
	 * @param ToTriangleID    The triangle being entered. Must share an edge with FromTriangleID.
	 *
	 * @return                The shared edge's midpoint, in world space.
	 */
	FVector GetSharedEdgeMidpoint(const int32 FromTriangleID, const int32 ToTriangleID) const;

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
	FVector CachedTangent1 = FVector::ZeroVector;
	FVector CachedTangent2 = FVector::ZeroVector;
};
