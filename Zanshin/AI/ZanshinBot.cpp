// Fill out your copyright notice in the Description page of Project Settings.

#include "Zanshin.h"
#include "ZanshinBot.h"
#include "ZanshinAIController.h"

AZanshinBot::AZanshinBot(const class FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	AIControllerClass = AZanshinAIController::StaticClass();
	CharacterTeam = 1;
}