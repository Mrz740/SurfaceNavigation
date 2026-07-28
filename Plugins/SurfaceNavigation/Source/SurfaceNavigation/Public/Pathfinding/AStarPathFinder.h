#pragma once

#include "CoreMinimal.h"
#include "AStarGraphAdapter.h"

bool SURFACENAVIGATION_API FindAStarPath(const IAStarGraphAdapter& Adapter, int32 StartNodeID, int32 GoalNodeID, TArray<int32>& OutPath);