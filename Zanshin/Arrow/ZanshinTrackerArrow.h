// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "Arrow/ZanshinBasicArrow.h"
#include "ZanshinTrackerArrow.generated.h"


/**
 * 
 */
UCLASS()
class ZANSHIN_API AZanshinTrackerArrow : public AZanshinBasicArrow
{
	GENERATED_BODY()

public:

	//////////////////////////////////////////////////////////////////////////
	// Constructor
	AZanshinTrackerArrow();

	UPROPERTY(EditAnywhere, Category = Tracker)
	float DeactivationDelayTracker;

	AZanshinCharacter* TrackedEnemy;

	//////////////////////////////////////////////////////////////////////////
	// Components

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = Particle)
	UParticleSystemComponent* TrackingParticleArrow;

	//////////////////////////////////////////////////////////////////////////
	// Particle on Enemy
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = Particle)
	UParticleSystemComponent* TrackingParticleOnEnemy;

protected:

	void SpecialPower(AZanshinCharacter* Enemy, const FHitResult& Hit) override;
	void SpecialPowerHitEnvironment(const FHitResult& Hit) override;
	void ActivatePassivePower(AZanshinCharacter* Enemy) override;
	void ResetTrackerEffect();

	//////////////////////////////////////////////////////////////////////////
	// Timer Handles

	UPROPERTY(EditAnywhere, Category = Trail)
	FTimerHandle DeactivateTrackerTimerHandle;


};
