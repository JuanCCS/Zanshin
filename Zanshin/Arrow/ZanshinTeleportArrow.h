// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "Arrow/ZanshinBasicArrow.h"
#include "ZanshinTeleportArrow.generated.h"

/**
 * 
 */
UCLASS()
class ZANSHIN_API AZanshinTeleportArrow : public AZanshinBasicArrow
{
	GENERATED_BODY()

public:

	//////////////////////////////////////////////////////////////////////////
	// Constructor

	AZanshinTeleportArrow();

	virtual void BeginPlay() override;

	UPROPERTY(EditAnywhere, Category = Gameplay)
	float DistanceBackwardOffset;

	UPROPERTY(EditAnywhere, Category = Gameplay)
	float DistanceUpdOffset;

	//////////////////////////////////////////////////////////////////////////
	// Particles

	UPROPERTY(EditAnywhere, Category = Particle)
	class UParticleSystem* ParticleTrailTP;

	UPROPERTY(EditAnywhere, Category = Particle)
	class UParticleSystem* TeleportCharacterParticle;

	UPROPERTY(EditAnywhere, Category = Particle)
	class UParticleSystem* TeleportCharacterTrace;

	//////////////////////////////////////////////////////////////////////////
	// Hit REsult
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Particle)
	FVector HitPoint;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Particle)
	FVector HitPointBefore;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Particle)
	bool bTeleported;

	//////////////////////////////////////////////////////////////////////////
	// Sound
	/** Sound to play each time we hit a headshot. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Gameplay)
	class USoundBase* TeleportSound;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Gameplay)
	TArray<FString> ParticleSys;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Gameplay)
	TArray<FString> ParticleParams;

	UFUNCTION(BlueprintImplementableEvent, meta = (displayname = "Teleport Burst"))
	void BP_Event_SpawnTeleportParticle(FVector DesiredSpawnLocation);

	UFUNCTION(BlueprintImplementableEvent, meta = (displayname = "Teleport Beam"))
	void BP_Event_SpawnBeam(FVector DesiredStart, FVector DesiredEnd);

	FVector SpawnLocation;

	virtual void UpdateCollision() override;

protected:

	void SpecialPower(AZanshinCharacter* Enemy, const FHitResult& Hit) override;
	void SpecialPowerHitEnvironment(const FHitResult& Hit) override;
	void ActivatePassivePower(AZanshinCharacter* Enemy) override;

private:

	void Teleport(const FHitResult& Hit);
};
