#pragma once

#include "CoreMinimal.h"
#include "SurfaceGraph.h"
#include "SurfacePath.h"

/**
 * Finds a walkable path across the surface graph, from a start position on one node to a goal
 * position on another.
 *
 * Runs two tiers of the same A* core. The macro tier searches the node graph for the sequence of
 * nodes to cross; the local tier then searches each of those nodes' own triangles in turn, so the
 * result is a polyline that stays on the surface rather than a sequence of node centers. Both tiers
 * go through FindAStarPath unchanged — the only difference between them is which IAStarGraphAdapter
 * is passed in.
 *
 * Every portal crossing produces two waypoints: an exit on the node being left and an entry on the
 * node being entered. For a welded seam those share a position, which is why FSurfacePath's contract
 * warns that consecutive waypoints may coincide. TransitionInfo is stamped on the arrival waypoint
 * only, and only when the crossed portal requires reorientation or bridges a gap.
 *
 * The caller supplies both node indices rather than having them located from the positions, matching
 * Detour's findPath: the movement component already knows which node it is standing on, and resolving
 * an arbitrary world point to a node is a separate concern with its own cost.
 *
 * @param Graph           The surface graph to search. Must not be mutated for the duration of the call.
 * @param StartPosition   World-space position to start from. Assumed to lie on StartNodeIndex.
 * @param StartNodeIndex  Node the start position sits on.
 * @param GoalPosition    World-space position to reach. Assumed to lie on GoalNodeIndex.
 * @param GoalNodeIndex   Node the goal position sits on.
 * @param OutPath         Receives the resulting waypoints. Reset on entry, and left empty on failure.
 *
 * @return                True when a path was found, false when no route exists between the two nodes
 *                        or the graph data is too malformed to search.
 */
bool SURFACENAVIGATION_API FindSurfacePath(const USurfaceGraph& Graph, const FVector& StartPosition, int32 StartNodeIndex,
	const FVector& GoalPosition, int32 GoalNodeIndex, FSurfacePath& OutPath);

