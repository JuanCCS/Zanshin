// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "Arrow/ZanshinBasicArrow.h"
#include "ZanshinExplosiveArrow.generated.h"

/**
 * 
 */
UCLASS()
class ZANSHIN_API AZanshinExplosiveArrow : public AZanshinBasicArrow
{
	GENERATED_BODY()
	
public:

	//////////////////////////////////////////////////////////////////////////
	// Constructor

	AZanshinExplosiveArrow();

	virtual void BeginPlay() override;

	//////////////////////////////////////////////////////////////////////////
	// Audio
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Audio)
	class USoundCue* ExplosionSound;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Audio)
	class USoundAttenuation* ExplosionSoundAtt;

	//////////////////////////////////////////////////////////////////////////
	// Particles

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = Particle)
	UParticleSystemComponent* ExplosiveParticle;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = Collider)
	USphereComponent* SphereCollider;
	
	UFUNCTION(BlueprintCallable, Category = "Special Power")
	void CreateExplosionRadius(const FHitResult& Hit);

	//////////////////////////////////////////////////////////////////////////
	// Camera Shake

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Shake)
	float ExplosionRadiusShake;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Shake)
	TSubclassOf<UCameraShake> ExplosionShake;

	///////////////////////////////////////////////////////////////////////
	// Collision Detection
	UFUNCTION()
	void OnOverlapBegin(class AActor* OtherActor, class UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

	UFUNCTION(BlueprintImplementableEvent, meta = (displayname = "Special Power"))
	void BP_Event_TriggerExplosion();

protected:

	///////////////////////////////////////////////////////////////////////
	// Effects

	UFUNCTION(BlueprintCallable, Category = "Special Power")
	void SpecialPower(AZanshinCharacter* Enemy, const FHitResult& Hit) override;
	
	UFUNCTION(BlueprintCallable, Category = "Special Power")
	void SpecialPowerHitEnvironment(const FHitResult& Hit) override;
	
	void ActivatePassivePower(AZanshinCharacter* Enemy) override;

	///////////////////////////////////////////////////////////////////////
	// Area of Effect Damage

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Projectile)
	float DamageExplosion;

	UPROPERTY()
	TArray<AZanshinCharacter*> Enemies;

	void DoAOEDamage();

private:

	bool bEndColision;
	bool bIsTimerSet;
	FTimerHandle TimerHandle_EndColision;

	void EndColision();
};