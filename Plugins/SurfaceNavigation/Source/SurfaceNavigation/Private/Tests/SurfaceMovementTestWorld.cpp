#include "SurfaceMovementTestWorld.h"
#include "Components/BoxComponent.h"

FSurfaceMovementTestWorld::FSurfaceMovementTestWorld()
{
	World = UWorld::CreateWorld(EWorldType::Game, false);
	FWorldContext& Context = GEngine->CreateNewWorldContext(EWorldType::Game);
	Context.SetCurrentWorld(World);
	World->InitializeActorsForPlay(FURL());
	World->BeginPlay();
}

FSurfaceMovementTestWorld::~FSurfaceMovementTestWorld()
{
	GEngine->DestroyWorldContext(World);
	World->DestroyWorld(false);
	World = nullptr;
	CollectGarbage(RF_NoFlags);
}

AActor* FSurfaceMovementTestWorld::SpawnFlatPrimitive(const FVector& Location, const FVector& Normal, const FVector& Extent) const
{
	AActor* Actor = World->SpawnActor<AActor>();
	
	UBoxComponent* Box = NewObject<UBoxComponent>(Actor);
	Box->SetBoxExtent(Extent);
	Box->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	Box->SetCollisionResponseToAllChannels(ECR_Block);
	Box->RegisterComponent();
	
	Actor->SetRootComponent(Box);
	Actor->SetActorLocation(Location);
	Actor->SetActorRotation(FQuat::FindBetweenNormals(FVector::UpVector, Normal).Rotator());
	return Actor;
}

AActor* FSurfaceMovementTestWorld::SpawnMovementActor(const FVector& Location) const
{
	AActor* Actor = World->SpawnActor<AActor>();
	
	USceneComponent* SceneComponent = NewObject<USceneComponent>(Actor);
	SceneComponent->RegisterComponent();
	
	Actor->SetRootComponent(SceneComponent);
	Actor->SetActorLocation(Location);
	Actor->SetActorRotation(FQuat::Identity.Rotator());
	
	USurfaceMovementComponent* MovementComponent = NewObject<USurfaceMovementComponent>(Actor);
	MovementComponent->RegisterComponent();

	return Actor;
}

void FSurfaceMovementTestWorld::TickWorld(float DeltaTime) const
{
	World->Tick(LEVELTICK_All, DeltaTime);
}
