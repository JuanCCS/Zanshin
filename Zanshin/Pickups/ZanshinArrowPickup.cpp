// Fill out your copyright notice in the Description page of Project Settings.

#include "Zanshin.h"
#include "ZanshinArrowPickup.h"
#include "Player/ZanshinCharacter.h"
#include "Bow/ZanshinBow.h"

AZanshinArrowPickup::AZanshinArrowPickup()
	:Super()	
{
	SphereToRotate = CreateDefaultSubobject<USphereComponent>(TEXT("SphereRotation"));
	SphereToRotate->AttachTo(Root);

	ArrowDisplay = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("ArrowComp"));
	ArrowDisplay->AttachTo(SphereToRotate);
}

void AZanshinArrowPickup::BeginPlay()
{
	Super::BeginPlay();

	UWorld* const World = GetWorld();
	if (World != nullptr) {
		FActorSpawnParameters Params;
		Params.Owner = this->GetOwner();
		Params.Instigator = Instigator;
		Deactivate();
	}
}

void AZanshinArrowPickup::Activate()
{
	Super::Activate();
	
	/*
	if (ArrowToDisplay != nullptr) {
		ArrowToDisplay->GetAnimationArrow()->SetVisibility(true);
	}
	*/
	if (SphereToRotate != nullptr)
	{
		SphereToRotate->SetVisibility(true,true);
	}

}

void AZanshinArrowPickup::Deactivate()
{
	Super::Deactivate();
	/*
	if (ArrowToDisplay != nullptr) {
		ArrowToDisplay->GetAnimationArrow()->SetVisibility(false);
	}
	*/
	if (SphereToRotate != nullptr)
	{
		SphereToRotate->SetVisibility(false,true);
	}
}

void AZanshinArrowPickup::ActiveEffect(class AActor* OtherActor)
{
	AZanshinCharacter* Player = Cast<AZanshinCharacter>(OtherActor);
	if (Player != nullptr && Player->Role == ROLE_Authority) {
		Player->AddArrow(ArrowToGive, AmmoToGive);
		Player->SetCurrentArrowAmmo();
	}
}