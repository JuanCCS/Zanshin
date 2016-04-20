// Fill out your copyright notice in the Description page of Project Settings.

#include "Zanshin.h"
#include "ZanshinNetRepCharacter.h"


// Sets default values
AZanshinNetRepCharacter::AZanshinNetRepCharacter(const class FObjectInitializer& ObjectInitializer)
{
 	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	bReplicates = true;
	bReplicateMovement = true;
	bNetLoadOnClient = true;

	NetPriority = DesiredNetPriority;
	

}

// Called when the game starts or when spawned
void AZanshinNetRepCharacter::BeginPlay()
{
	Super::BeginPlay();
	
}

// Called every frame
void AZanshinNetRepCharacter::Tick( float DeltaTime )
{
	Super::Tick( DeltaTime );
}

// Called to bind functionality to input
void AZanshinNetRepCharacter::SetupPlayerInputComponent(class UInputComponent* InputComponent)
{
	Super::SetupPlayerInputComponent(InputComponent);

}

// For one-off Functions That Take no value and return no value. With no failure probability.
bool AZanshinNetRepCharacter::ROS_Function_Validate()
{
	return true;
}

void AZanshinNetRepCharacter::ROS_Function_Implementation()
{

}