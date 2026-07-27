#pragma once

#include "CoreMinimal.h"
#include "SurfaceGraph.h"

struct FTwoNodeDiagonalSquareFixture 
{
	USurfaceGraph* Graph;
	int32 NodeAIndex, NodeBIndex;
	int32 EdgeIndex;
	int32 Idx0, Idx1, Idx2, Idx3;
};                                                                                            
                                                                                                
namespace SurfaceGraphTestUtils
{
	FTwoNodeDiagonalSquareFixture BuildTwoNodeDiagonalSquareGraph();
};
