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

struct FDecoyCycleGraphFixture
{
	USurfaceGraph* Graph;
	int32 NodeSIndex;
	int32 NodeM1Index;
	int32 NodeM2Index;
	int32 NodeM3Index;
	int32 NodeGIndex;
};

namespace SurfaceGraphTestUtils
{
	FTwoNodeDiagonalSquareFixture BuildTwoNodeDiagonalSquareGraph();
	FDecoyCycleGraphFixture BuildDecoyCycleGraph();
	int32 AddIsolatedNode(USurfaceGraph& Graph);

	int32 AddTriangleNode(USurfaceGraph& Graph, int32 V0, int32 V1, int32 V2, const FVector& Normal);
	int32 AddPortalEdge(USurfaceGraph& Graph, int32 NodeAIndex, int32 SlotA, int32 NodeBIndex, int32 SlotB, bool bIsGapBridge, bool bRequiresReorientation);
};
