// Fill out your copyright notice in the Description page of Project Settings.

#include "Zanshin.h"
#include "ZanshinGameModeTeamDeathMatch.h"

AZanshinGameModeTeamDeathMatch::AZanshinGameModeTeamDeathMatch(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer)
{
	NumTeams = 2;
}

void AZanshinGameModeTeamDeathMatch::InitGameState()
{
	Super::InitGameState();

	CurrentGameState = Cast<AZanshinGameState>(GameState);
	if (CurrentGameState) {
		CurrentGameState->NumTeams = NumTeams;
	}
}

void AZanshinGameModeTeamDeathMatch::DetermineMatchWinner()
{
	int32 BestScore = MAX_uint32;
	int32 BestTeam = -1;
	int32 NumBestTeams = 1;

	WinnerTeam = (NumBestTeams == 1) ? BestTeam : NumTeams;
}