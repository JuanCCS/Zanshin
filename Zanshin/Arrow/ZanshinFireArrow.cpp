// Fill out your copyright notice in the Description page of Project Settings.

#include "Zanshin.h"
#include "ZanshinFireArrow.h"

AZanshinFireArrow::AZanshinFireArrow() : Super()
{
	FireParticleArrow = CreateDefaultSubobject<UParticleSystemComponent>(TEXT("Fire Particle"));
	FireParticleArrow->AttachTo(TipSphereComponent);

	KillTypeArrow = ArrowKillType::FIRE;
}

///////////////////////////////////////////////////////////////////////
// Effects

void AZanshinFireArrow::SpecialPower(AZanshinCharacter* Enemy, const FHitResult& Hit)
{
	Enemy->SetParticle(ParticleEffectOnCharacter);
	Enemy->SlowDown(AmountToSlowDownMultiplier);
}

void AZanshinFireArrow::SpecialPowerHitEnvironment(const FHitResult& Hit) { }

void AZanshinFireArrow::ActivatePassivePower(AZanshinCharacter* Enemy)
{
}