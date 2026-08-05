#include "SurfaceMovementTestWorld.h"
#include "Components/BoxComponent.h"
#include "Movement/SurfaceMovementComponent.h"

#if WITH_DEV_AUTOMATION_TESTS

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
	Box->SetCollisionObjectType(ECC_WorldStatic);
	Box->RegisterComponent();
	
	Actor->SetRootComponent(Box);
	Actor->SetActorLocation(Location);
	Actor->SetActorRotation(FQuat::FindBetweenNormals(FVector::UpVector, Normal).Rotator());
	return Actor;
}

AActor* FSurfaceMovementTestWorld::SpawnMovementActor(const FVector& Location) const
{
	AActor* Actor = World->SpawnActor<AActor>();
	
	UBoxComponent* Box = NewObject<UBoxComponent>(Actor);
	Box->SetBoxExtent(FVector(5.f, 5.f, 5.f));
	Box->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	Box->SetCollisionResponseToAllChannels(ECR_Ignore);
	Box->SetCollisionResponseToChannel(ECC_WorldStatic, ECR_Block);
	Box->RegisterComponent();
	
	Actor->SetRootComponent(Box);
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

#endif
