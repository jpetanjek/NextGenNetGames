// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Buildable.h"
#include "BuildableManager.generated.h"

UCLASS()
class GDKSHOOTER_API ABuildableManager : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	ABuildableManager();
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	UFUNCTION(BlueprintCallable)
	void SpawnRequest(TSubclassOf<class ABuildable> BuildableFortification,FVector const& Location, FRotator const& Rotation);

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	TArray<ABuildable*> managedBuildables;

};
