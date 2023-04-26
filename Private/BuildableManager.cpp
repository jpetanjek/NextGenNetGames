// Copyright (c) Improbable Worlds Ltd, All Rights Reserved


#include "BuildableManager.h"

// Sets default values
ABuildableManager::ABuildableManager()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

}

// Called when the game starts or when spawned
void ABuildableManager::BeginPlay()
{
	Super::BeginPlay();
	
}

// Called every frame
void ABuildableManager::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

void ABuildableManager::SpawnRequest(TSubclassOf<class ABuildable> BuildableFortification, FVector const& Location, FRotator const& Rotation) {
	FActorSpawnParameters SpawnParams;
	SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;


	ABuildable* tempBuildable = GetWorld()->SpawnActor<ABuildable>(BuildableFortification, Location, Rotation, SpawnParams);
	tempBuildable->Place();
	//tempBuildable->Build(90);
	GEngine->AddOnScreenDebugMessage(-1, 15, FColor::Green, TEXT("ABuildableManager::SpawnRequest"));
	managedBuildables.Add(tempBuildable);
}

