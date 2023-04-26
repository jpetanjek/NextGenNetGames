// Copyright (c) Improbable Worlds Ltd, All Rights Reserved


#include "BuildManagerComponent.h"
#include "Buildable.h"
#include "Runtime/Engine/Classes/Kismet/GameplayStatics.h"
#include "GameFramework/Actor.h"
#include "BuildableManager.h"

// Sets default values for this component's properties
UBuildManagerComponent::UBuildManagerComponent()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = true;

	isBuilding = false;
	canBuild = false;
	previewMode = true;

	// ...
}


// Called when the game starts
void UBuildManagerComponent::BeginPlay()
{
	Super::BeginPlay();
	playerCamera = GetOwner()->FindComponentByClass<UCameraComponent>();
	if (playerCamera == nullptr) {
		//UE_LOG(LogTemp, Error, Text("Unable to find player camera"));
	}
	// ...
	
}


// Called every frame
void UBuildManagerComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	FRotator CameraRotation1 = playerCamera->GetComponentRotation();
	FVector Start1 = playerCamera->GetComponentLocation() + (CameraRotation1.Vector() * 150.f);

	FVector end = Start1 + (CameraRotation1.Vector() * 20000.f);

	//GEngine->AddOnScreenDebugMessage(-1, 15.0f, FColor::Purple, FString::FromInt(end.X) + " " + FString::FromInt(end.Y) + " " + FString::FromInt(end.Z));

	if (isBuilding) {

		FHitResult HitResult;

		float LineTraceDistance = 1800.f;
		float HeadOffset = 150.f;

		FRotator CameraRotation = playerCamera->GetComponentRotation();
		FVector Start = playerCamera->GetComponentLocation() + (CameraRotation.Vector() * HeadOffset);

		FVector End = Start + (CameraRotation.Vector() * LineTraceDistance);

		FCollisionQueryParams TraceParams(FName(TEXT("InteractTrace")), true, NULL);
		TraceParams.bTraceComplex = true;
		TraceParams.bReturnPhysicalMaterial = true;

		bool bIsHit = GetWorld()->LineTraceSingleByChannel(
			HitResult,      // FHitResult object that will be populated with hit info
			Start,      // starting position
			End,        // end position
			ECC_Visibility,  // collision channel
			TraceParams      // additional trace settings
		);

		currentTrace = HitResult.ImpactPoint;

		//DrawPreview
		if (bIsHit&&previewMode){
			// start to end, green, will lines always stay on, depth priority, thickness of line
			//DrawDebugLine(GetWorld(), Start, HitResult.ImpactPoint, FColor::Green, false, 5.f, ECC_WorldStatic, 1.f);

			FRotator ProperRotation = HitResult.GetComponent()->GetComponentRotation();
			ProperRotation.SetComponentForAxis(EAxis::Z, CameraRotation.GetComponentForAxis(EAxis::Z));

			FVector ProperLocation = HitResult.ImpactPoint;
			

			FActorSpawnParameters SpawnParams;
			SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;

			if (currentBuildable == nullptr) {
				currentBuildable = GetWorld()->SpawnActor<ABuildable>(BuildableFortification, ProperLocation, ProperRotation, SpawnParams);
				//DrawDebugBox(GetWorld(), ProperLocation, FVector(100, 100, 100), ProperRotation.Quaternion(), FColor::Red, false, 0, 0, 10);
			}
			else {
				currentBuildable->SetActorLocationAndRotation(ProperLocation, ProperRotation);
			}
			canBuild = true;
		}else if (bIsHit&&!previewMode){
			FVector distanceCalculator = currentTrace - plantingPoint;
			DrawDebugLine(GetWorld(), plantingPoint, currentTrace, FColor::Green, false, 5.f, ECC_WorldStatic, 1.f);
			float distance = distanceCalculator.Size();

			/*
			FString TheFloatStr = FString::SanitizeFloat(distance);
			GEngine->AddOnScreenDebugMessage(-1, 15, FColor::Green, *TheFloatStr);
			*/

			//Get buildable lenght
			FVector debugSize = currentBuildable->GetComponentsBoundingBox(true,true).GetExtent();
			
			//Divide distance between planting point and currentTrace by buildable lenght
			int amountOfCover = (distance / debugSize.Size())+1;

			//For each full distance spawn a buidable and add it to managedBuildables
			FActorSpawnParameters SpawnParams;
			SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;
			if (managedBuildables.Num() < amountOfCover) {
				for (int i = managedBuildables.Num(); i < amountOfCover; i++) {
					FVector additiveVector = plantingPoint + (distanceCalculator.GetSafeNormal() * (debugSize.Size() * i));
					managedBuildables.Add(GetWorld()->SpawnActor<ABuildable>(BuildableFortification, additiveVector, distanceCalculator.Rotation(), SpawnParams));
				}
			}

			//Destroy extra buildables
			if (managedBuildables.Num() > amountOfCover) {
				for (int i = amountOfCover; i < managedBuildables.Num(); i++) {
					managedBuildables[i]->Destroy();
					managedBuildables.RemoveAt(i);
				}
			}

			//Fix rotation in real time
			if (managedBuildables.Num() > 0) {
				for (int i = 0; i < managedBuildables.Num(); i++) {
					FVector additiveVector = plantingPoint + (distanceCalculator.GetSafeNormal() * (debugSize.Size() * i));

					

					//Shoot trace up
					FVector endVectorCheck = additiveVector + (FVector(0, 0, 1) * 2000.f);
					FHitResult HitResult1;

					bool traceUp = GetWorld()->LineTraceSingleByChannel(
						HitResult1,      // FHitResult object that will be populated with hit info
						additiveVector,      // starting position
						endVectorCheck,        // end position
						ECC_Visibility,  // collision channel
						TraceParams      // additional trace settings
					);

					//Shoot trace down
					FVector endVectorCheck2 = additiveVector + (FVector(0, 0, -1) * 10000.f);
					FHitResult HitResult2;
					FVector startVector2;
					if (traceUp) {
						startVector2 = HitResult1.ImpactPoint;
					}
					else {
						startVector2 = endVectorCheck;
					}
					bool traceDown = GetWorld()->LineTraceSingleByChannel(
						HitResult2,      // FHitResult object that will be populated with hit info
						startVector2,      // starting position
						endVectorCheck2,        // end position
						ECC_Visibility,  // collision channel
						TraceParams      // additional trace settings
					);

					//If something is hit while going down, place the object there, else destroy it
					FVector spawnVector;
					if (traceDown) {
						spawnVector = HitResult2.ImpactPoint;
						FRotator finRotation = FRotator(spawnVector.Rotation().Pitch, distanceCalculator.Rotation().Yaw, spawnVector.Rotation().Roll);
						managedBuildables[i]->SetActorLocationAndRotation(spawnVector, finRotation);
					}
					else {
						managedBuildables[i]->Destroy();
						managedBuildables.RemoveAt(i);
					}
				}
			}
		}
		else if (!bIsHit) {
			canBuild = false;
		}
	}
	else if(currentBuildable != nullptr) {
		currentBuildable->Destroy();
		currentBuildable = nullptr;
	}

}

void UBuildManagerComponent::ToggleBuildMode() {
	isBuilding = !isBuilding;
	GEngine->AddOnScreenDebugMessage(-1,15,FColor::Green,TEXT("ToogleBuildMode"));
	if (!canBuild || !isBuilding) {
		for (int i = 0; i < managedBuildables.Num(); i++) {
			managedBuildables[i]->Destroy();
		}
	}
	managedBuildables.Empty();
}

void UBuildManagerComponent::RequestBuild() {
	if (canBuild) {
		GEngine->AddOnScreenDebugMessage(-1, 15, FColor::Green, TEXT("RequestBuild"));
		previewMode = false;
		currentBuildable->PreviewMesh1->ToggleVisibility();
		plantingPoint = currentTrace;
	}
}

void UBuildManagerComponent::ReleaseBuild() {
	currentBuildable->Destroy();
	currentBuildable = nullptr;


	if (canBuild && isBuilding) {
		for (int i = 0; i < managedBuildables.Num(); i++) {
			Server_PlaceBuildable(managedBuildables[i]->GetActorLocation(), managedBuildables[i]->GetActorRotation());
			managedBuildables[i]->Destroy();
		}
	}

	if (!canBuild || !isBuilding) {
		for (int i = 0; i < managedBuildables.Num(); i++) {
			managedBuildables[i]->Destroy();
		}
	}
	managedBuildables.Empty();
		
	previewMode = true;
}

bool UBuildManagerComponent::Server_PlaceBuildable_Validate(FVector const& Location, FRotator const& Rotation) {
	return true;
}

void UBuildManagerComponent::Server_PlaceBuildable_Implementation(FVector const& Location, FRotator const& Rotation) {
	
	TArray<AActor*> FoundActors;
	UGameplayStatics::GetAllActorsOfClass(GetWorld(), ABuildableManager::StaticClass(), FoundActors);
	ABuildableManager* foundManager = Cast<ABuildableManager>(FoundActors[0]);
	
	GEngine->AddOnScreenDebugMessage(-1, 15, FColor::Green, TEXT("UBuildManagerComponent::Server_PlaceBuildable_Implementation"));

	foundManager->SpawnRequest(BuildableFortification, Location, Rotation);
}

