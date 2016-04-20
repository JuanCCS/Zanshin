// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "Pickups/ZanshinBasicPickup.h"
#include "ZanshinHealthPickup.generated.h"

/**
 * 
 */
UCLASS()
class ZANSHIN_API AZanshinHealthPickup : public AZanshinBasicPickup
{
	GENERATED_BODY()
	
public:

	AZanshinHealthPickup();
	
	UPROPERTY(EditAnywhere, Category = Health)
	int16 LifeToGive;

	//////////////////////////////////////////////////////////////////////////
	// Audio
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Audio)
	class USoundCue* HealthSound;

protected:

	void ActiveEffect(class AActor* OtherActor) override;
};
