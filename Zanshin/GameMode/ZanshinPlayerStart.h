// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "GameFramework/PlayerStart.h"
#include "ZanshinPlayerStart.generated.h"

/**
 * 
 */
UCLASS()
class ZANSHIN_API AZanshinPlayerStart : public APlayerStart
{
	GENERATED_UCLASS_BODY()

	/** Which team can start at this point */
	UPROPERTY(EditInstanceOnly, Category = Team)
	int32 SpawnTeam;

	UPROPERTY(EditAnywhere, Category = Spawn)
	int32 SpawnCount = 0;
};