// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Engine.h"
#include "BuildManagerComponent.generated.h"



UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class GDKSHOOTER_API UBuildManagerComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	UBuildManagerComponent();

	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	UFUNCTION(BlueprintCallable)
	void ToggleBuildMode();

	UFUNCTION(BlueprintCallable)
	void RequestBuild();

	UFUNCTION(BlueprintCallable)
		void ReleaseBuild();

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Building)
		TSubclassOf<class ABuildable> BuildableFortification;

	UFUNCTION(Server, reliable, WithValidation, BlueprintCallable)
		void Server_PlaceBuildable(FVector const& Location, FRotator const& Rotation);

protected:
	// Called when the game starts
	virtual void BeginPlay() override;

private:

	bool isBuilding;
	bool canBuild;
	bool previewMode;

	FVector plantingPoint;
	FVector currentTrace;

	UCameraComponent *playerCamera;

	ABuildable *currentBuildable;

	TArray<ABuildable*> managedBuildables;
};
