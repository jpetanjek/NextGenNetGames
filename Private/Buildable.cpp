// Copyright (c) Improbable Worlds Ltd, All Rights Reserved


#include "Buildable.h"
#include "Components/BoxComponent.h"
#include "Engine/CollisionProfile.h"
#include <Runtime\Engine\Public\Net\UnrealNetwork.h>

// Sets default values
ABuildable::ABuildable()
{
	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	bReplicates = true;

	TestTag1 = CreateDefaultSubobject<UTestTag>(TEXT("TestTag1"));

	HealthComponent = CreateDefaultSubobject<UHealthComponent>(TEXT("Health"));
	HealthComponent->SetIsReplicated(true);

	RootComponent = CreateDefaultSubobject<USceneComponent>(TEXT("RootComponent"));
	RootComponent->SetIsReplicated(true);

	PreviewMesh1 = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("PreviewMesh"));
	PreviewMesh1->SetupAttachment(RootComponent);
	PreviewMesh1->SetVisibility(true);
	PreviewMesh1->SetCollisionProfileName(UCollisionProfile::NoCollision_ProfileName);
	PreviewMesh1->SetIsReplicated(true);


	BuildMesh1 = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("BuildMesh1"));
	BuildMesh1->SetupAttachment(RootComponent);
	BuildMesh1->SetVisibility(false);
	BuildMesh1->SetCollisionProfileName(UCollisionProfile::NoCollision_ProfileName);
	BuildMesh1->SetIsReplicated(true);

	BuildMesh2 = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("BuildMesh2"));
	BuildMesh2->SetupAttachment(RootComponent);
	BuildMesh2->SetVisibility(false);
	BuildMesh2->SetCollisionProfileName(UCollisionProfile::NoCollision_ProfileName);
	BuildMesh2->SetIsReplicated(true);

	BuildMesh3 = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("BuildMesh3"));
	BuildMesh3->SetupAttachment(RootComponent);
	BuildMesh3->SetVisibility(false);
	BuildMesh3->SetCollisionProfileName(UCollisionProfile::NoCollision_ProfileName);
	BuildMesh3->SetIsReplicated(true);

	collision.Init(false, 3);
}

// Called when the game starts or when spawned
void ABuildable::BeginPlay()
{
	Super::BeginPlay();
	
}

void ABuildable::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ABuildable,collision);
}

// Called every frame
void ABuildable::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

void ABuildable::Place() {
	PreviewMesh1->SetVisibility(false);
	BuildMesh1->SetVisibility(true);
		
	BuildMesh1->SetCollisionProfileName(UCollisionProfile::BlockAll_ProfileName);
	collision[0] = true;

	
	HealthComponent->GrantHealth((1 / 10) * HealthComponent->GetMaxHealth());
	GEngine->AddOnScreenDebugMessage(-1, 15, FColor::Green, TEXT("ABuildable::Place"));
}

void ABuildable::OnRep_Collision() {
	if (collision[0]) {
		BuildMesh1->SetCollisionProfileName(UCollisionProfile::BlockAll_ProfileName);
	}
	else {
		BuildMesh1->SetCollisionProfileName(UCollisionProfile::NoCollision_ProfileName);
	}

	if (collision[1]) {
		BuildMesh2->SetCollisionProfileName(UCollisionProfile::BlockAll_ProfileName);
	}
	else {
		BuildMesh2->SetCollisionProfileName(UCollisionProfile::NoCollision_ProfileName);
	}

	if (collision[2]) {
		BuildMesh3->SetCollisionProfileName(UCollisionProfile::BlockAll_ProfileName);
	}
	else {
		BuildMesh3->SetCollisionProfileName(UCollisionProfile::NoCollision_ProfileName);
	}
}


void ABuildable::Build(float value) {
	GEngine->AddOnScreenDebugMessage(-1, 15, FColor::Green, TEXT("ABuildable::Build"));
	HealthComponent->GrantHealth(value);
	HelathUpdate();
}

void ABuildable::HelathUpdate() {
	if (HealthComponent->GetCurrentHealth() >= 0.35 * HealthComponent->GetMaxHealth() && HealthComponent->GetCurrentHealth() <= 0.75 * HealthComponent->GetMaxHealth()) {
		BuildMesh2->SetVisibility(true);
		collision[1] = true;
		BuildMesh2->SetCollisionProfileName(UCollisionProfile::BlockAll_ProfileName);

		BuildMesh1->SetVisibility(false);
		collision[0] = false;
		BuildMesh1->SetCollisionProfileName(UCollisionProfile::NoCollision_ProfileName);

		BuildMesh3->SetVisibility(false);
		collision[2] = false;
		BuildMesh3->SetCollisionProfileName(UCollisionProfile::NoCollision_ProfileName);
	}
	else if (HealthComponent->GetCurrentHealth() > 0.75 * HealthComponent->GetMaxHealth()) {
		BuildMesh3->SetVisibility(true);
		collision[2] = true;
		BuildMesh3->SetCollisionProfileName(UCollisionProfile::BlockAll_ProfileName);

		BuildMesh1->SetVisibility(false);
		collision[0] = false;
		BuildMesh1->SetCollisionProfileName(UCollisionProfile::NoCollision_ProfileName);

		BuildMesh2->SetVisibility(false);
		collision[1] = false;
		BuildMesh2->SetCollisionProfileName(UCollisionProfile::NoCollision_ProfileName);
	}
	else if (HealthComponent->GetCurrentHealth() == 0) {
		this->Destroy();
	}
}

float ABuildable::TakeDamage(float Damage, const FDamageEvent& DamageEvent, AController* EventInstigator, AActor* DamageCauser)
{
	TakeDamageCrossServer(Damage, DamageEvent, EventInstigator, DamageCauser);
	return Damage;
}

void ABuildable::TakeDamageCrossServer_Implementation(float Damage, const FDamageEvent& DamageEvent, AController* EventInstigator, AActor* DamageCauser)
{
	float ActualDamage = Super::TakeDamage(Damage, DamageEvent, EventInstigator, DamageCauser);
	HealthComponent->TakeDamage(ActualDamage, DamageEvent, EventInstigator, DamageCauser);
	HelathUpdate();
}
