// Copyright (c) Mrz740. Licensed under the MIT License.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Object.h"
#include "SurfaceGraph.generated.h"

USTRUCT()
struct FSurfaceHoleLoop
{
	GENERATED_BODY()
	
	UPROPERTY()
	TArray<int32> Indices;
};

USTRUCT()
struct FSurfaceGraphNode
{
	GENERATED_BODY()
	
	UPROPERTY()
	TArray<int32> TriangleVertexIndices;
	UPROPERTY()
	TArray<int32> OuterBoundaryIndices;
	UPROPERTY()
	TArray<FSurfaceHoleLoop> HoleBoundaryIndices;
	UPROPERTY()
	TArray<int32> NeighborEdgeIndices;
	UPROPERTY()
	FVector Normal = FVector::ZeroVector;
};

USTRUCT()
struct FSurfaceGraphEdge
{
	GENERATED_BODY()
	
	UPROPERTY()
	int32 NodeAIndex = INDEX_NONE;
	UPROPERTY()
	int32 NodeBIndex = INDEX_NONE;
	UPROPERTY()
	int32 PortalSlotA = INDEX_NONE;
	UPROPERTY()
	int32 PortalSlotB = INDEX_NONE;
	UPROPERTY()
	bool bRequiresReorientation = false;
	UPROPERTY()
	bool bIsGapBridge = false;
};

enum class ELoopContainment : uint8
{
	Outside,
	OnBoundary,
	Inside
};

UCLASS()
class SURFACENAVIGATION_API USurfaceGraph : public UObject
{
	GENERATED_BODY()
	
	UPROPERTY()
	TArray<FVector> SharedVertices;
	UPROPERTY()
	TArray<FSurfaceGraphNode> Nodes;
	UPROPERTY()
	TArray<FSurfaceGraphEdge> Edges;

public:
	const FSurfaceGraphNode* GetNode(const int32 NodeIndex) const;
	const FSurfaceGraphEdge* GetEdge(const int32 EdgeIndex) const;
	const FVector* GetVertex(const int32 VertexIndex) const;
	
	bool AddNeighborEdgeToNode(const int32 NodeIndex, const int32 EdgeIndex);
	bool IsPointInPolygon(const FSurfaceGraphNode& Node, const FVector& Point) const;
	
	static void BuildPlaneBasis(const FVector& Normal, FVector& OutTangent1, FVector& OutTangent2);
	
	ELoopContainment TestPointAgainstLoop(TArrayView<const int32> LoopIndices, const FVector& Point, const FVector& Tangent1, const FVector& Tangent2) const;

	// Single line getters, setters and adders without logic
	int32 GetNodeCount() const { return Nodes.Num(); }
	int32 GetEdgeCount() const { return Edges.Num(); }
	int32 GetSharedVertexCount() const { return SharedVertices.Num(); }

	int32 AddNode(const FSurfaceGraphNode& Node) { return Nodes.Add(Node); }
	int32 AddEdge(const FSurfaceGraphEdge& Edge) { return Edges.Add(Edge); }
	int32 AddVertex(const FVector& Vertex) { return SharedVertices.Add(Vertex); }
	
private:
	static FVector2D Project2D(const FVector& Point3D, const FVector& PlaneOrigin, const FVector& Tangent1, const FVector& Tangent2);
	static double DistancePointToSegment2D(const FVector2D& P, const FVector2D& A, const FVector2D& B);
};
