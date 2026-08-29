// Copyright (c) Mrz740. Licensed under the MIT License.


#include "Bake/SurfaceBakeVolume.h"
#include "ActorFactories/ActorFactory.h"
#include "Builders/CubeBuilder.h"
#include "Components/BrushComponent.h"
#include "Engine/CollisionProfile.h"
#include "SurfaceNavigationEditor.h"
#include "EngineUtils.h"
#include "PhysicsEngine/BodySetup.h"

namespace
{
	struct FSurfaceTriangleSoup
	{
		TArray<FVector> Positions;
		TArray<FTriIndices> Triangles;
	};

	FSurfaceTriangleSoup HarvestVolume(const ASurfaceBakeVolume& Volume)
	{
		FSurfaceTriangleSoup TriangleSoup;
		int TotalKept = 0;
		int TotalDiscarded = 0;
		int SkippedContributors = 0;

		for (AActor* Actor : TActorRange<AActor>(Volume.GetWorld()))
		{
			for (TInlineComponentArray<UStaticMeshComponent*> Components(Actor); UStaticMeshComponent* Comp : Components)
			{
				if (Comp->Bounds.GetBox().Intersect(Volume.GetBounds().GetBox()))
				{
					UStaticMesh* Mesh = Comp->GetStaticMesh();
					if (Mesh == nullptr) continue;
					UBodySetup* Body = Mesh->GetBodySetup();
					if (Body == nullptr) continue;

					if (Body->GetCollisionTraceFlag() != CTF_UseComplexAsSimple)
					{
						UE_LOG(LogSurfaceBake, Warning, TEXT("Skipping %s: collision trace flag is not CTF_UseComplexAsSimple"), *Actor->GetName())
						SkippedContributors++;
						continue;
					}

					if (!Mesh->ContainsPhysicsTriMeshData(true)) continue;

					FTriMeshCollisionData Data;

					if (!Mesh->GetPhysicsTriMeshData(&Data, true)) continue;

					FTransform Xf = Comp->GetComponentTransform();
					bool bFlip = Data.bFlipNormals != (Xf.GetDeterminant() < 0);
					int Kept = 0;
					int Discarded = 0;

					for (FTriIndices TriIndices : Data.Indices)
					{
						FVector P0 = Xf.TransformPosition(FVector(Data.Vertices[TriIndices.v0]));
						FVector P1 = Xf.TransformPosition(FVector(Data.Vertices[TriIndices.v1]));
						FVector P2 = Xf.TransformPosition(FVector(Data.Vertices[TriIndices.v2]));

						if (!Volume.EncompassesPoint((P0 + P1 + P2) / 3))
						{
							Discarded++;
							continue;
						}

						int BaseIndex = TriangleSoup.Positions.Num();
						TriangleSoup.Positions.Append({ P0, P1, P2 });
						FTriIndices NewTri;
						NewTri.v0 = BaseIndex;

						if (bFlip)
						{
							NewTri.v1 = BaseIndex + 2;
							NewTri.v2 = BaseIndex + 1;
						}
						else
						{
							NewTri.v1 = BaseIndex + 1;
							NewTri.v2 = BaseIndex + 2;
						}

						TriangleSoup.Triangles.Add(NewTri);
						Kept++;
					}

					UE_LOG(LogSurfaceBake, Display, TEXT("%s: kept %d, discarded %d"), *Actor->GetName(), Kept, Discarded)
					TotalKept += Kept;
					TotalDiscarded += Discarded;
				}
			}
		}

		UE_LOG(LogSurfaceBake, Display, TEXT("Harvest complete: kept %d, discarded %d, skipped contributors %d"), TotalKept, TotalDiscarded, SkippedContributors)
		return TriangleSoup;
	}
}

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
	HarvestVolume(*this);
}

void ASurfaceBakeVolume::PostActorCreated()
{
	Super::PostActorCreated();

	if (Brush == nullptr)
	{
		UActorFactory::CreateBrushForVolumeActor(this, NewObject<UCubeBuilder>());
	}
}
