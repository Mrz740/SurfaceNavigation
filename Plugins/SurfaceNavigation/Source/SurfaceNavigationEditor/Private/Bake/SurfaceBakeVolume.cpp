// Copyright (c) Mrz740. Licensed under the MIT License.


#include "Bake/SurfaceBakeVolume.h"
#include "ActorFactories/ActorFactory.h"
#include "Builders/CubeBuilder.h"
#include "Components/BrushComponent.h"
#include "Engine/CollisionProfile.h"
#include "SurfaceNavigationEditor.h"

ASurfaceBakeVolume::ASurfaceBakeVolume()
{
	GetBrushComponent()->SetCollisionProfileName(UCollisionProfile::NoCollision_ProfileName);
	GetBrushComponent()->Mobility = EComponentMobility::Static;

	BrushColor = FColor(200, 200, 200, 255);

	bColored = true;
	bIsSpatiallyLoaded = false;
}

void ASurfaceBakeVolume::Bake()
{
	UE_LOG(LogSurfaceBake, Display, TEXT("Bake button works."))
}

void ASurfaceBakeVolume::PostActorCreated()
{
	Super::PostActorCreated();

	if (Brush == nullptr)
	{
		UActorFactory::CreateBrushForVolumeActor(this, NewObject<UCubeBuilder>());
	}
}
