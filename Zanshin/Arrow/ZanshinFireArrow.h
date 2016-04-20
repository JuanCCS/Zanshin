// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "Arrow/ZanshinBasicArrow.h"
#include "ZanshinFireArrow.generated.h"

/**
 * 
 */
UCLASS()
class ZANSHIN_API AZanshinFireArrow : public AZanshinBasicArrow
{
	GENERATED_BODY()
	
public:

	//////////////////////////////////////////////////////////////////////////
	// Constructor
	AZanshinFireArrow();

	//////////////////////////////////////////////////////////////////////////
	// Components

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = Particle)
	UParticleSystemComponent* FireParticleArrow;

	//////////////////////////////////////////////////////////////////////////
	// Particle on Enemy
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = Particle)
	UParticleSystemComponent* FireParticleCharacter;

	UPROPERTY(EditAnywhere, Category = Gameplay)
	float AmountToSlowDownMultiplier;

protected:

	void SpecialPower(AZanshinCharacter* Enemy, const FHitResult& Hit) override;
	void SpecialPowerHitEnvironment(const FHitResult& Hit) override;
	void ActivatePassivePower(AZanshinCharacter* Enemy) override;
	
};
