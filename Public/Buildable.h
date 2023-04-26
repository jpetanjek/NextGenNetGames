// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Characters/Components/HealthComponent.h"
#include "TestTag.h"
#include "Buildable.generated.h"


UCLASS()
class GDKSHOOTER_API ABuildable : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	ABuildable();

	UPROPERTY(Category = Character, VisibleAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
		UHealthComponent *HealthComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Building)
	UStaticMeshComponent *PreviewMesh1;


	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Building)
		UStaticMeshComponent *BuildMesh1;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Building)
		UStaticMeshComponent *BuildMesh2;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Building)
		UStaticMeshComponent *BuildMesh3;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Building)
		class UTestTag* TestTag1;

	UPROPERTY(ReplicatedUsing = OnRep_Collision)
		TArray<bool> collision;

	UFUNCTION()
	void OnRep_Collision();

	UFUNCTION(BlueprintCallable, Category = Building)
		void Place();

	UFUNCTION(BlueprintCallable, Category = Building)
		void Build(float value);

	UFUNCTION(BlueprintCallable, Category = Health)
		void HelathUpdate();


	// Called every frame
	virtual void Tick(float DeltaTime) override;

	float TakeDamage(float Damage, const struct FDamageEvent& DamageEvent, AController* EventInstigator, AActor* DamageCauser) override;

	UFUNCTION(CrossServer, Reliable)
		void TakeDamageCrossServer(float Damage, const struct FDamageEvent& DamageEvent, AController* EventInstigator, AActor* DamageCauser);
	
protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

};
