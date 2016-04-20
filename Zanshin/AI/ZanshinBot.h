// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "Player/ZanshinCharacter.h"
#include "ZanshinBot.generated.h"

/**
*
*/
UCLASS()
class ZANSHIN_API AZanshinBot : public AZanshinCharacter
{
	GENERATED_UCLASS_BODY()

public:

	UPROPERTY(EditAnywhere, Category = Behavior)
	class UBehaviorTree* BehaviorTree;
};