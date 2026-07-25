// Copyright (c) Mrz740. Licensed under the MIT License.


#include "SurfaceGraph.h"

const FSurfaceGraphNode* USurfaceGraph::GetNode(const int32 NodeIndex) const
{
	if (Nodes.IsValidIndex(NodeIndex))
	{
		return &Nodes[NodeIndex];
	}
	return nullptr;
}

const FSurfaceGraphEdge* USurfaceGraph::GetEdge(const int32 EdgeIndex) const
{
	if (Edges.IsValidIndex(EdgeIndex))
	{
		return &Edges[EdgeIndex];
	}
	return nullptr;
}

bool USurfaceGraph::AddNeighborEdgeToNode(const int32 NodeIndex, const int32 EdgeIndex)
{
	if (Nodes.IsValidIndex(NodeIndex) && Edges.IsValidIndex(EdgeIndex))
	{
		Nodes[NodeIndex].NeighborEdgeIndices.Add(EdgeIndex);
		return true;
	}
	return false;
}

bool USurfaceGraph::IsPointInPolygon(const FSurfaceGraphNode& Node, const FVector& Point) const
{
	FVector ReferenceAxis;

	const float AbsX = FMath::Abs(Node.Normal.X);
	const float AbsY = FMath::Abs(Node.Normal.Y);
	const float AbsZ = FMath::Abs(Node.Normal.Z);
	
	if (AbsX <= AbsY && AbsX <= AbsZ) {ReferenceAxis = FVector::XAxisVector;}
	else if (AbsY <= AbsZ) {ReferenceAxis = FVector::YAxisVector;}
	else {ReferenceAxis = FVector::ZAxisVector;}

	const FVector Tangent1 = FVector::CrossProduct(ReferenceAxis,Node.Normal).GetSafeNormal();
	const FVector Tangent2 = FVector::CrossProduct(Node.Normal,Tangent1).GetSafeNormal();

	const FVector PlaneOrigin = SharedVertices[Node.OuterBoundaryIndices[0]];
	const FVector2D Point2D = Project2D(Point,PlaneOrigin,Tangent1,Tangent2);

	if (IsPointOnLoopBoundary(Node.OuterBoundaryIndices,Point2D,PlaneOrigin,Tangent1,Tangent2,KINDA_SMALL_NUMBER)) return true;
	if (!TestLoop(Node.OuterBoundaryIndices,Point2D,PlaneOrigin,Tangent1,Tangent2)) return false;
	
	for (int32 i = 0; i < Node.HoleBoundaryIndices.Num(); i++)
	{
		if (IsPointOnLoopBoundary(Node.HoleBoundaryIndices[i].Indices,Point2D,PlaneOrigin,Tangent1,Tangent2,KINDA_SMALL_NUMBER)) return true;
		if (TestLoop(Node.HoleBoundaryIndices[i].Indices,Point2D,PlaneOrigin,Tangent1,Tangent2)) return false;
	}
	return true;
}

bool USurfaceGraph::IsPointOnLoopBoundary(const TArray<int32>& LoopIndices, const FVector2D& Point2D,
	const FVector& PlaneOrigin, const FVector& Tangent1, const FVector& Tangent2, float Epsilon) const
{
	for (int32 i = 0; i < LoopIndices.Num(); i++)
	{
		const int32 j = (i - 1 + LoopIndices.Num()) % LoopIndices.Num();

		const FVector2D Vi = Project2D(SharedVertices[LoopIndices[i]],PlaneOrigin,Tangent1,Tangent2);
		const FVector2D Vj = Project2D(SharedVertices[LoopIndices[j]],PlaneOrigin,Tangent1,Tangent2);
		
		if (DistancePointToSegment2D(Point2D,Vi,Vj) <= Epsilon) return true;
	}
	return false;
}

bool USurfaceGraph::TestLoop(const TArray<int32>& LoopIndices, const FVector2D& Point2D, const FVector& PlaneOrigin, const FVector& Tangent1, const FVector& Tangent2) const
{
	bool bInside = false;

	for (int32 i = 0; i < LoopIndices.Num(); i++)
	{
		const int32 j = (i - 1 + LoopIndices.Num()) % LoopIndices.Num();
		const FVector2D Vi = Project2D(SharedVertices[LoopIndices[i]],PlaneOrigin,Tangent1,Tangent2);
		const FVector2D Vj = Project2D(SharedVertices[LoopIndices[j]],PlaneOrigin,Tangent1,Tangent2);
		
		if (!(Vi.Y > Point2D.Y) != (Vj.Y > Point2D.Y)) continue;
		if (Point2D.X <= (Vi.X + (Vj.X - Vi.X) * (Point2D.Y - Vi.Y) / (Vj.Y - Vi.Y))) bInside = !bInside;
	}
	return bInside;
}

FVector2D USurfaceGraph::Project2D(const FVector& Point3D, const FVector& PlaneOrigin, const FVector& Tangent1,
                                   const FVector& Tangent2)
{
	const FVector RelativePoint = Point3D - PlaneOrigin;
	return FVector2D(RelativePoint | Tangent1, RelativePoint | Tangent2);
}

double USurfaceGraph::DistancePointToSegment2D(const FVector2D& P, const FVector2D& A, const FVector2D& B)
{
	const FVector2D AB = B - A;
	double t = FVector2D::DotProduct(P - A,AB) / FVector2D::DotProduct(AB,AB);
	t = FMath::Clamp(t,0.0,1.0);
	return FVector2D((P - A) - t * AB).Length();
}
