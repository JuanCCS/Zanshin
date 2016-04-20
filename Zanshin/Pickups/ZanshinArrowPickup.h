// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "Pickups/ZanshinBasicPickup.h"
#include "ZanshinArrowPickup.generated.h"

/**
 * 
 */
UCLASS()
class ZANSHIN_API AZanshinArrowPickup : public AZanshinBasicPickup
{
	GENERATED_BODY()
	
public:

	//////////////////////////////////////////////////////////////////////////
	// Constructor

	AZanshinArrowPickup();

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Arrow)
	TSubclassOf<class AZanshinBasicArrow> ArrowToGive;

	UPROPERTY(EditAnywhere, Category = Arrow)
	int16 AmmoToGive;

	//////////////////////////////////////////////////////////////////////////
	// Audio

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Gameplay)
	class USoundCue* AmmoPickup;

	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	virtual void Activate() override;
	virtual void Deactivate() override;

protected:

	class AZanshinBasicArrow* ArrowToDisplay;

	void ActiveEffect(class AActor* OtherActor) override;

	UPROPERTY(VisibleDefaultsOnly, Category = Pickup)
	class UStaticMeshComponent* ArrowDisplay;

	UPROPERTY(VisibleDefaultsOnly, Category = Pickup)
	class USphereComponent* SphereToRotate;
};