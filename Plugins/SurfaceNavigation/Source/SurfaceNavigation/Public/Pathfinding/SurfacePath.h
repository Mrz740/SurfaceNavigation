#pragma once

#include "CoreMinimal.h"

/**
 * Describes a surface transition, carried by the waypoint the agent arrives at.
 *
 * Present only on crossings that need special handling by the movement layer — that is, when the
 * crossed portal has bRequiresReorientation or bIsGapBridge set. A welded seam between two
 * co-oriented nodes carries none, because it is behaviorally identical to a crossing between two
 * triangles inside a single node: no gap to span and no change of orientation.
 *
 * The two flags are carried separately rather than combined into one, because the movement layer
 * needs to tell them apart when selecting a position curve, even though either one alone is enough
 * to mark the crossing as a transition.
 */
struct FSurfaceTransitionInfo
{
	/** Surface normal of the node being left. */
	FVector DepartureNormal = FVector::ZeroVector;

	/** Surface normal of the node being entered. */
	FVector ArrivalNormal = FVector::ZeroVector;

	/** True when the two surfaces differ enough in orientation to require a reorientation maneuver. */
	bool bRequiresReorientation = false;

	/** True when the portal spans a physical gap rather than a welded seam. */
	bool bIsGapBridge = false;
};

/**
 * A single position along a surface path, together with the node it lies on.
 *
 * Absence of a transition is expressed with TOptional rather than a zeroed struct plus a boolean,
 * so that "no transition here" can never be confused with a transition whose fields happen to hold
 * default values.
 */
struct FSurfaceWaypoint
{
	/** World-space position of this waypoint. */
	FVector Position = FVector::ZeroVector;

	/** Index of the surface graph node this waypoint lies on. */
	int32 NodeIndex = INDEX_NONE;

	/** Set only when arriving across a portal that requires reorientation or bridges a gap. */
	TOptional<FSurfaceTransitionInfo> TransitionInfo;
};

/**
 * An ordered polyline produced by a surface path query.
 *
 * Consumer contract: consecutive waypoints may share the same Position. This occurs at every welded
 * seam, where the departing and arriving waypoints occupy one point on two different nodes, and for
 * a request whose start and goal positions are identical. Consumers must therefore not assume a
 * non-zero direction between consecutive waypoints — a normalized direction across such a pair is a
 * zero vector. Testing TransitionInfo is not sufficient to detect these pairs, since a welded seam
 * between two co-oriented nodes carries none.
 *
 * Holds nothing but the waypoints: the length is Waypoints.Num(), and success or failure is carried
 * by the return value of the query that produced the path.
 *
 * These three types are plain structs rather than USTRUCTs because nothing currently needs
 * reflection — they are transient per-request output, never serialized, replicated, or edited. If a
 * UFUNCTION ever has to carry a waypoint across a reflected boundary, promoting them is additive:
 * add the generated header include, USTRUCT(), GENERATED_BODY() and UPROPERTY(). Note that TOptional
 * cannot cross into Blueprint, so a BlueprintCallable accessor would need TransitionInfo rethought.
 */
struct FSurfacePath
{
	/** Ordered waypoints from start to goal. Empty when the query that produced it failed. */
	TArray<FSurfaceWaypoint> Waypoints;
};
