// Copyright (c) Mrz740. Licensed under the MIT License.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Volume.h"
#include "SurfaceBakeVolume.generated.h"

/** Geometry parameters consumed by the bake core. Output and debug settings live on the volume instead, since the core neither saves nor draws. */
USTRUCT()
struct FSurfaceBakeParams
{
	GENERATED_BODY()

	/**
	 * Distance below which two positions are welded into a single shared vertex.
	 * With no CSG union, this is the only mechanism connecting geometry authored as separate meshes.
	 */
	UPROPERTY(EditAnywhere, Category= "SurfaceBake|Tolerances", meta = (ClampMin = 0.f, Units = "cm"))
	float VertexWeldTolerance = 0.5f;
	/** Minimum dot product against a group's seed normal for a triangle to join that node, where 1.0 requires exact parallelism */
	UPROPERTY(EditAnywhere, Category= "SurfaceBake|Tolerances", meta = (ClampMin = 0.f, ClampMax = 1.f))
	float CoplanarNormalTolerance = 0.999f;
	/** Distance a triangle may sit off its group's seed plane and still join, which is what separates a shelf from the floor below it */
	UPROPERTY(EditAnywhere, Category= "SurfaceBake|Tolerances", meta = (ClampMin = 0.f, Units = "cm"))
	float CoplanarDistanceTolerance = 0.1f;
	/** Distance that defines how far a GapBridge will reach */
	UPROPERTY(EditAnywhere, Category= "SurfaceBake|Tolerances", meta = (ClampMin = 0.f, Units = "cm"))
	float MaxGapBridgeDistance = 50.f;
	/** Threshold that defines what counts as a corner needing a transition arc, where the default is roughly 45 degrees */
	UPROPERTY(EditAnywhere, Category= "SurfaceBake|Tolerances", meta = (ClampMin = 0.f, ClampMax = 1.f))
	float ReorientationNormalThreshold = 0.7f;
	/** Width below which a portal is rejected as too narrow to traverse, where zero disables the filter */
	UPROPERTY(EditAnywhere, Category= "SurfaceBake|Tolerances", meta = (ClampMin = 0.f, Units = "cm"))
	float MinPortalWidth = 5.f;
};

/**
 * Editor-only volume defining the region of a level baked into a surface navigation graph.
 *
 * Geometry inside its bounds contributes and everything outside is ignored, following the precedent of
 * ANavMeshBoundsVolume. Contributing meshes are read through their complex collision rather than their
 * render geometry, so the baked graph and the runtime surface probe agree about where surfaces are.
 */
UCLASS(HideCategories = (Collision, HLOD, Cooking, Replication, Navigation, Networking, Actor, Rendering, LOD,
	LevelInstance, Streaming, Input, DataLayers, WorldPartition), PrioritizeCategories = "SurfaceBake")
class SURFACENAVIGATIONEDITOR_API ASurfaceBakeVolume : public AVolume
{
	GENERATED_BODY()

public:
	ASurfaceBakeVolume();

	/** Geometry parameters handed to the bake core */
	UPROPERTY(EditAnywhere, Category= "SurfaceBake", meta = (ShowOnlyInnerProperties))
	FSurfaceBakeParams BakeParams;
	/** Content directory the baked graph asset is written into */
	UPROPERTY(EditAnywhere, Category= "SurfaceBake|Output", meta = (ContentDir))
	FDirectoryPath OutputAssetPath;
	/** Name given to the baked graph asset within the output directory */
	UPROPERTY(EditAnywhere, Category= "SurfaceBake|Output")
	FString OutputAssetName;
	/** Draws the baked graph's nodes, normals, and portals into the editor viewport without entering play */
	UPROPERTY(EditAnywhere, Category= "SurfaceBake")
	bool bDrawDebugGraph = false;

	/** Bakes the geometry inside this volume into a surface navigation graph asset */
	UFUNCTION(CallInEditor, Category= "SurfaceBake")
	void Bake();

	/**
	 * Gives a newly placed volume its default cube brush.
	 *
	 * Stock volumes get their geometry from UActorFactoryBoxVolume, but that factory's NewActorClass is
	 * AVolume itself, so FindActorFactoryByClassForActorClass rejects it for any subclass and the Place
	 * Actors palette falls back to plain class placement. That path never builds a brush, leaving
	 * UBrushComponent::CreateSceneProxy with a null Brush and nothing to draw.
	 */
	virtual void PostActorCreated() override;
};
