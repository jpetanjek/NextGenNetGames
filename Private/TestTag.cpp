// Copyright (c) Improbable Worlds Ltd, All Rights Reserved


#include "TestTag.h"

// Sets default values for this component's properties
UTestTag::UTestTag()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = true;

	// ...
}


// Called when the game starts
void UTestTag::BeginPlay()
{
	Super::BeginPlay();

	// ...
	
}


// Called every frame
void UTestTag::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	// ...
}

