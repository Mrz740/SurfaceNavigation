#pragma once

#include "CoreMinimal.h"
#include "AStarGraphAdapter.h"
#include "SurfaceGraph.h"

class FSurfaceGraphAStarAdapter final : public IAStarGraphAdapter
{
public:
	~FSurfaceGraphAStarAdapter() = default;
	explicit FSurfaceGraphAStarAdapter(const USurfaceGraph& InGraph);

	virtual void GetNeighbors(int32 NodeID, TArray<int32>& OutNeighbors) const override;
	virtual float GetCost(int32 FromNodeID, int32 ToNodeID) const override;
	virtual float GetHeuristic(int32 FromNodeID, int32 GoalNodeID) const override;
	
private:
	const USurfaceGraph& SurfaceGraph;
	TArray<FVector> NodeCenters;
	TArray<FVector> PortalMidpointsA;
	TArray<FVector> PortalMidpointsB;
	
	// Helper functions
	FVector ResolvePortalMidpoint(int32 NodeIndex, int32 Slot) const;
};
