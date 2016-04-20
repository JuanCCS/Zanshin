// Fill out your copyright notice in the Description page of Project Settings.

#include "Zanshin.h"
#include "ZanshinTrackerArrow.h"

AZanshinTrackerArrow::AZanshinTrackerArrow() : Super()
{
	KillTypeArrow = ArrowKillType::TRACKER;
}

///////////////////////////////////////////////////////////////////////
// Effects

void AZanshinTrackerArrow::SpecialPower(AZanshinCharacter* Enemy, const FHitResult& Hit)
{
	Enemy->GetMesh()->SetRenderCustomDepth(true);
	Enemy->GetMesh()->SetCustomDepthStencilValue(Enemy->GetTeam());
	
	TrackedEnemy = Enemy;
	
	UWorld* World = GetWorld();
	if (World) {
		World->GetTimerManager().SetTimer(DeactivateTrackerTimerHandle, this, &AZanshinTrackerArrow::ResetTrackerEffect, DeactivationDelayTracker);
	}
}

void AZanshinTrackerArrow::SpecialPowerHitEnvironment(const FHitResult& Hit) { }
void AZanshinTrackerArrow::ActivatePassivePower(AZanshinCharacter* Enemy) { }

void AZanshinTrackerArrow::ResetTrackerEffect()
{
	if (TrackedEnemy) {
		TrackedEnemy->GetMesh()->SetRenderCustomDepth(false);
		TrackedEnemy->DeactivatedNextArrowKill();
	}
}