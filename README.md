# SurfaceNavigation

[![Quality gate status](https://sonarcloud.io/api/project_badges/measure?project=Mrz740_SurfaceNavigation2&metric=alert_status)](https://sonarcloud.io/summary/new_code?id=Mrz740_SurfaceNavigation2)

A from-scratch surface-based (floor/wall/ceiling) navigation and movement system for Unreal
Engine 5, built as a standalone, plug-and-play plugin.

Instead of a NavMesh, walkable surfaces are baked into a graph of polygon nodes connected by
portal edges. Pathfinding runs in two tiers — macro A* across nodes, then local refinement
within a node — and agents move with surface-projected orientation (floor, wall, or ceiling)
rather than a fixed up-vector.

## Status

Early bootstrap — the plugin skeleton compiles (Runtime + Editor modules), no gameplay code yet.
Implementation is in progress; a full architecture writeup, design-decision rationale, and demo
will land here as milestones complete.

## License

See [LICENSE](LICENSE).