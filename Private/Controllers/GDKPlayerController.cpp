// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#include "Controllers/GDKPlayerController.h"

#include "Blueprint/UserWidget.h"
#include "Camera/CameraComponent.h"
#include "Controllers/Components/ControllerEventsComponent.h"
#include "Characters/Components/EquippedComponent.h"
#include "Characters/Components/HealthComponent.h"
#include "Characters/Components/MetaDataComponent.h"
#include "EngineClasses/SpatialNetDriver.h"
#include "Interop/Connection/SpatialWorkerConnection.h"
#include "Game/Components/ScorePublisher.h"
#include "Game/Components/SpawnRequestPublisher.h"
#include "Game/Components/PlayerPublisher.h"
#include "GameFramework/Character.h"
#include "GameFramework/GameStateBase.h"
#include "GameFramework/PlayerState.h"
#include "GameFramework/SpringArmComponent.h"
#include "GameFramework/TouchInterface.h"
#include "Net/UnrealNetwork.h"
#include "Weapons/Holdable.h"
#include "Weapons/Projectile.h"
#include "Weapons/Weapon.h"
#include "Interop/SpatialSender.h"
#include "DrawDebugHelpers.h"

AGDKPlayerController::AGDKPlayerController()
	: bIgnoreActionInput(false)
	, DeleteCharacterDelay(5.0f)
{
	// Don't automatically switch the camera view when the pawn changes, to avoid weird camera jumps when a character dies.
	bAutoManageActiveCameraTarget = false;

	// Create a camera boom (pulls in towards the controller if there is a collision)
	CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
	CameraBoom->SetupAttachment(RootComponent);
	CameraBoom->TargetArmLength = 300.0f; // The camera follows at this distance behind the character	
	CameraBoom->bUsePawnControlRotation = true; // Rotate the arm based on the controller

	// Create the third person camera
	DeathCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("DeathCamera"));
	DeathCamera->SetupAttachment(CameraBoom, USpringArmComponent::SocketName); // Attach the camera to the end of the boom and let the boom adjust to match the controller orientation
	DeathCamera->bUsePawnControlRotation = false; // Camera does not rotate relative to arm

	////START

	//Setup Actor Interest Component
	ActorInterestComponent = CreateDefaultSubobject<UActorInterestComponent>(TEXT("Actor Interest"));
	ActorInterestComponent->bUseNetCullDistanceSquaredForCheckoutRadius = true;

	//Setup Query
	FQueryData query;
	

	////

	//[0] - Setup TeamTag query test - 200m can see enemy, but not friendly
	
	RelativeSphere = CreateDefaultSubobject<URelativeSphereConstraint>(TEXT("RelativeSphere"));
	RelativeSphere->Radius = 10000;

	Actor1 = CreateDefaultSubobject<UActorClassConstraint>(TEXT("Actor1"));
	Actor1->ActorClass = APawn::StaticClass();
	Actor1->bIncludeDerivedClasses = true;

	PlayerBubbleComponent = CreateDefaultSubobject<UComponentClassConstraint>(TEXT("PlayerBubbleComponent"));
	PlayerBubbleComponent->ComponentClass = UTagComponent::StaticClass();
	PlayerBubbleComponent->bIncludeDerivedClasses = true;

	PlayerBubbleAnd = CreateDefaultSubobject<UAndConstraint>(TEXT("PlayerBubbleAnd"));
	PlayerBubbleAnd->Constraints.Add(PlayerBubbleComponent);
	PlayerBubbleAnd->Constraints.Add(Actor1);
	PlayerBubbleAnd->Constraints.Add(RelativeSphere);

	query.Constraint = PlayerBubbleAnd;
	query.Frequency = 60;

	//Add query to Actor Interest Component
	ActorInterestComponent->Queries.Add(query);
	

	///
	
	//[1] - Setup 1. Player bubble query - 50m at top frequency
	PlayerBubble = CreateDefaultSubobject<UCheckoutRadiusConstraint>(TEXT("PlayerBubble"));
	PlayerBubble->ActorClass = APawn::StaticClass();
	PlayerBubble->Radius = 5000;
	
	query.Constraint = PlayerBubble;
	query.Frequency = 30;

	//Add query to Actor Interest Component
	ActorInterestComponent->Queries.Add(query);
		
	////

	//[2] - Setup 1. FOV Constraint query - 100m at top frequency
	FOVConstraint1 = CreateDefaultSubobject<UAndConstraint>(TEXT("FOVConstraint1"));

	Sphere1 = CreateDefaultSubobject<USphereConstraint>(TEXT("Sphere1"));
	Sphere1->Radius = 4000;
	Sphere1->Center = FVector::ZeroVector;

	FOVConstraint1->Constraints.Add(Sphere1);
	FOVConstraint1->Constraints.Add(Actor1);

	query.Constraint = FOVConstraint1;
	query.Frequency = 60;

	//Add query to Actor Interst Component
	ActorInterestComponent->Queries.Add(query);

	////

	////END

	//Initialize counter for update speed - workaround because we are calling directly in to the net driver without any other management
	updateCounter = 0;
}

void AGDKPlayerController::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (GetPawn())
	{
		LatestPawnYaw = GetPawn()->GetActorRotation().Yaw;
	}
}

void AGDKPlayerController::SetPawn(APawn* InPawn)
{
	Super::SetPawn(InPawn);

	if (GetNetMode() == NM_Client && InPawn)
	{
		SetViewTarget(InPawn);
		// Make the new pawn's camera this controller's camera.
		this->ClientSetRotation(InPawn->GetActorRotation(), true);
	}
	else
	{
		SetViewTarget(this);
	}

	PawnEvent.Broadcast(InPawn);
	OnNewPawn(InPawn);
}

void AGDKPlayerController::QueryTest()
{
	FVector vector;
	FRotator rotator;
	FVector end;

	GetPlayerViewPoint(vector, rotator);

	//1. FOV 100
	end = vector + (rotator.Vector() * 4000.f);
	//GEngine->AddOnScreenDebugMessage(-1, 15, FColor::Yellow, end.ToString());

	Sphere1->Center = end;

	FOVConstraint1->Constraints.Empty();
	FOVConstraint1->Constraints.Add(Sphere1);
	FOVConstraint1->Constraints.Add(Actor1);

	ActorInterestComponent->Queries[2].Constraint=FOVConstraint1;
	
	//End FOV
	
	//Update team component - call this seperately not every second
	AGDKCharacter* charOwned = (AGDKCharacter*)GetPawn();
	UTeamComponent* tcomp = charOwned->FindComponentByClass<UTeamComponent>();
	if (tcomp->GetTeam()==0) {
		//Is red team, get interst on Blue team
		RelativeSphere->Radius = 15000;
		PlayerBubbleComponent->ComponentClass = UTagComponent::StaticClass();
		PlayerBubbleComponent->bIncludeDerivedClasses = true;
		//GEngine->AddOnScreenDebugMessage(-1, 15, FColor::Blue, charOwned->GetName() + " " + FString::FromInt(tcomp->GetTeam()));

	}
	else {
		RelativeSphere->Radius = 20000;
		PlayerBubbleComponent->ComponentClass = UTagComponent::StaticClass();
		PlayerBubbleComponent->bIncludeDerivedClasses = true;
		//GEngine->AddOnScreenDebugMessage(-1, 15, FColor::Red, charOwned->GetName() + " " + FString::FromInt(tcomp->GetTeam()));
	}
	

	PlayerBubbleAnd->Constraints.Empty();
	PlayerBubbleAnd->Constraints.Add(PlayerBubbleComponent);
	PlayerBubbleAnd->Constraints.Add(Actor1);
	PlayerBubbleAnd->Constraints.Add(RelativeSphere);

	ActorInterestComponent->Queries[0].Constraint = PlayerBubbleAnd;
	

	USpatialNetDriver* driver =(Cast<USpatialNetDriver>(GetWorld()->GetNetDriver()));
	USpatialSender* sender = driver->Sender;
	sender->UpdateInterestComponent(this);	
	
}

void AGDKPlayerController::GetPlayerViewPoint(FVector& out_Location, FRotator& out_Rotation) const
{
	if (!GetPawn())
	{
		out_Location = DeathCamera->GetComponentLocation();
		out_Rotation = DeathCamera->GetComponentRotation();
		out_Rotation.Add(0, LatestPawnYaw, 0);
	}
	else
	{
		Super::GetPlayerViewPoint(out_Location, out_Rotation);
	}
}

void AGDKPlayerController::SetControlRotation(const FRotator& NewRotation)
{
	Super::SetControlRotation(NewRotation);
	updateCounter++;
	if (GetLocalRole() == ROLE_Authority && updateCounter%60==0)
	{
		// Networked client in control.
		QueryTest();
		updateCounter = 0;
	}
}

void AGDKPlayerController::SetUIMode(bool bIsUIMode)
{
	bShowMouseCursor = bIsUIMode;
	ResetIgnoreLookInput();
	SetIgnoreLookInput(bIsUIMode);
	ResetIgnoreMoveInput();
	SetIgnoreMoveInput(bIsUIMode);

	if (bIsUIMode)
	{
		SetInputMode(FInputModeGameAndUI());
		ActivateTouchInterface(nullptr);
	}
	else
	{
		SetInputMode(FInputModeGameOnly());
		CreateTouchInterface();
	}

	if (GetPawn())
	{
		if (UEquippedComponent* EquippedComponent = Cast<UEquippedComponent>(GetPawn()->GetComponentByClass(UEquippedComponent::StaticClass())))
		{
			EquippedComponent->BlockUsing(bIsUIMode);
		}
	}
}

void AGDKPlayerController::ServerTryJoinGame_Implementation()
{

	if (USpawnRequestPublisher* Spawner = Cast<USpawnRequestPublisher>(GetWorld()->GetGameState()->GetComponentByClass(USpawnRequestPublisher::StaticClass())))
	{
		Spawner->RequestSpawn(this);
		return;
	}
}

bool AGDKPlayerController::ServerTryJoinGame_Validate()
{
	return true;
}

void AGDKPlayerController::ServerRequestName_Implementation(const FString& NewPlayerName)
{
	if (PlayerState)
	{
		PlayerState->SetPlayerName(NewPlayerName);
	}
}

bool AGDKPlayerController::ServerRequestName_Validate(const FString& NewPlayerName)
{
	return true;
}

void AGDKPlayerController::ServerRequestMetaData_Implementation(const FGDKMetaData NewMetaData)
{
	if (UMetaDataComponent* MetaData = Cast<UMetaDataComponent>(PlayerState->GetComponentByClass(UMetaDataComponent::StaticClass())))
	{
		MetaData->SetMetaData(NewMetaData);
	}
}

bool AGDKPlayerController::ServerRequestMetaData_Validate(const FGDKMetaData NewMetaData)
{
	return true;
}

void AGDKPlayerController::ServerRespawnCharacter_Implementation()
{
	if (USpawnRequestPublisher* Spawner = Cast<USpawnRequestPublisher>(GetWorld()->GetGameState()->GetComponentByClass(USpawnRequestPublisher::StaticClass())))
	{
		Spawner->RequestSpawn(this);
		return;
	}
}

bool AGDKPlayerController::ServerRespawnCharacter_Validate()
{
	return true;
}

