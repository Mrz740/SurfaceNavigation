// Copyright (c) Mrz740. Licensed under the MIT License.

#pragma once

#include "CoreMinimal.h"

/**
 * An interface for providing graph data to the A* pathfinding algorithm.
 *
 * One implementation per search tier: the macro tier walks the surface graph's nodes across portal
 * edges, the local tier walks the triangles inside a single node. FindPath is written once against
 * this interface and never learns which of the two it is talking to.
 *
 * Implementations are borrowed, not owned — FindPath takes a reference for the duration of a single
 * call. They are expected to be cheap, short-lived objects, typically stack-allocated by the caller.
 * Nothing in this design deletes an adapter through this interface, which is why the destructor is
 * protected and non-virtual.
 */
class SURFACENAVIGATION_API IAStarGraphAdapter
{
public:
	/**
	 * Collects the IDs of every node directly adjacent to NodeID.
	 *
	 * The pathfinder calls this once per node expansion and passes the same array every time, so
	 * implementations must call OutNeighbors.Reset() before adding anything — never Empty(), which
	 * frees the capacity that makes reusing the array allocation-free.
	 * The array belongs to the caller and must not be retained beyond this call.
	 *
	 * @param NodeID        The node whose neighbors are wanted.
	 * @param OutNeighbors  On return, holds exactly NodeID's neighbors and nothing else.
	 */
	virtual void GetNeighbors(int32 NodeID, TArray<int32>& OutNeighbors) const = 0;

	/**
	 * Both nodes must always be adjacent and the cost between them must be non-negative so A* and
	 * Dijkstra do not break. The pathfinder only ever passes a node and one of its own neighbors, so
	 * implementations do not need to handle arbitrary pairs.
	 *
	 * The pair is ordered, so asymmetric costs are allowed: moving floor-to-wall may legitimately
	 * cost more than wall-to-floor.
	 *
	 * @param FromNodeID  The node from which the cost is calculated.
	 * @param ToNodeID    The adjacent node to which the cost is calculated.
	 * 
	 * @return            The exact traversal cost between those two nodes. Never negative.
	 */
	virtual float GetCost(int32 FromNodeID, int32 ToNodeID) const = 0;

	/**
	 * Unlike GetCost this is an estimate, and it is called with arbitrary pairs that are usually not
	 * adjacent — an implementation cannot answer it by looking up an edge.
	 *
	 * The estimate must never exceed the true remaining cost (it must be admissible), and for
	 * adjacent nodes A and B it must satisfy h(A) <= GetCost(A, B) + h(B) (it must be consistent).
	 * Consistency matters because the search closes a node permanently the first time it is expanded,
	 * so a merely-admissible estimate can close one too early. Breaking either property fails
	 * silently: FindPath still returns a valid, traversable path, it is simply no longer guaranteed
	 * to be the shortest one.
	 *
	 * Returning 0 is always safe — it degrades the search to Dijkstra, which explores more nodes but
	 * stays optimal. Straight-line distance between node positions satisfies both properties.
	 *
	 * @param FromNodeID  The node from which the heuristic is calculated.
	 * @param GoalNodeID  The goal node towards which the heuristic is calculated.
	 * 
	 * @return            An underestimate of the remaining cost towards the goal. Never negative.
	 */
	virtual float GetHeuristic(int32 FromNodeID, int32 GoalNodeID) const = 0;

protected:
	~IAStarGraphAdapter() = default;
};
