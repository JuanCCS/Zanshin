// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "GameMode/ZanshinGameMode.h"
#include "ZanshinGameState.h"
#include "ZanshinGameModeTeamDeathMatch.generated.h"

/**
 * 
 */

UCLASS()
class ZANSHIN_API AZanshinGameModeTeamDeathMatch : public AZanshinGameMode
{
	GENERATED_BODY()


public:
	//////////////////////////////////////////////////////////////////////////
	// Constructor

	AZanshinGameModeTeamDeathMatch(const FObjectInitializer& ObjectInitializer);

protected:
	/** number of teams */
	int32 NumTeams;

	/** best team */
	int32 WinnerTeam;

	/** check who won */
	virtual void DetermineMatchWinner() override;

private:

	AZanshinGameState* CurrentGameState;

	/** initialize replicated game data */
	virtual void InitGameState() override;

	void BroadcastControlPointScore();

public:

	
};