// Fill out your copyright notice in the Description page of Project Settings.

#include "Zanshin.h"
#include "ZanshinHealthPickup.h"
#include "Player/ZanshinCharacter.h"

AZanshinHealthPickup::AZanshinHealthPickup()
{

}

void AZanshinHealthPickup::ActiveEffect(class AActor* OtherActor)
{
	AZanshinCharacter* Player = Cast<AZanshinCharacter>(OtherActor);
	if (Player) 
	{
		Player->AddHealth(LifeToGive);
	}
}