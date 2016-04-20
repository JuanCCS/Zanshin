// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "GameFramework/GameState.h"
#include "GameMode/ZanshinGameMode.h"
#include "Player/ZanshinPlayerState.h"
#include "ZanshinGameState.generated.h"

/**
 * 
 */

/** Used to display a player's info on the UI. */
USTRUCT(BlueprintType)
struct FZanshinPlayerNetInfo
{
	GENERATED_USTRUCT_BODY()

		FZanshinPlayerNetInfo(){
	}

	/** Player chosen username */
	UPROPERTY(BlueprintReadWrite)
		FString NetPlayerName;

	/** Player's Kills */
	UPROPERTY(BlueprintReadWrite)
		int32 NetPlayerKills;

	/** Player's Kills */
	UPROPERTY(BlueprintReadWrite)
		int32 NetPlayerAssists;

	/** Player's Deaths */
	UPROPERTY(BlueprintReadWrite)
		int32 NetPlayerDeaths;

	/** Player's Current Score */
	UPROPERTY(BlueprintReadWrite)
		int32 NetPlayerScore;
};

USTRUCT(BlueprintType)
struct FControlPointData
{
	GENERATED_USTRUCT_BODY()

	/** The number of the team that captured the point. */
	UPROPERTY(BlueprintReadWrite)
	int32 TeamNumber;

	/** The letter of the control point. */
	UPROPERTY(BlueprintReadWrite)
	int32 CapturedPointLetter;

	/** The amount of point to give to the team that captured the control points. */
	UPROPERTY(BlueprintReadWrite)
	int32 PointsToGive;

	/** Defaults. */
	FControlPointData()
	{
		TeamNumber = 0;
		PointsToGive = 0;
	}
};

UCLASS()
class ZANSHIN_API AZanshinGameState : public AGameState
{
	GENERATED_BODY()

public:

	AZanshinGameState();

	virtual void BeginPlay() override;
	
	virtual void Tick(float DeltaTime) override;

	/** number of teams in current game (doesn't deprecate when no players are left in a team) */
	UPROPERTY(Transient, Replicated)
	int32 NumTeams;
	
	/** These two hold the team informaiton.
	*	[Server] Contains all PlayerStates and the correct Score.
	*	[Client] Contains all PlayerStates. 
	*/
	FZanshinTeam BlueTeam;
	FZanshinTeam RedTeam;

	TArray<AActor*> ControlPoints;

	//////////////////////////////////////////////////////////////////////////
	//	Lobby

	/** For the Lobby UI. Number of players who have pressed ready. */
	UPROPERTY(BlueprintReadOnly, ReplicatedUsing = OnRep_ReadyPlayers, Category = NetworkState)
	int32 NumberOfReadyPlayers = 0;

	/** UI.  Called when a player presses ready. Notifies all clients. */
	UFUNCTION(BlueprintImplementableEvent, meta = (DisplayName = "Update_ReadyPlayers"))
	void OnRep_ReadyPlayers();

	//////////////////////////////////////////////////////////////////////////
	//	Game

	/** UI. Broadcasted Score Info to all Clients. */
	UFUNCTION(BlueprintImplementableEvent, meta = (DisplayName = "Update_TeamScores"))
	void BP_Event_UpdateTeamScores(FZanshinScoreInfo ScoreInfo);

	//////////////////////////////////////////////////////////////////////////
	//	Control Point

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Gameplay)
	int32 NumberOfControlPoints;

	int32 ControlPointsScore = 20;

	float UpdateControlPointInterval = 5.f;
	float ControlPointsTimer = 0.f;

	void CheckControlPoints(float DeltaTime);

	void BroadcastControlPointsScore();

	UFUNCTION(NetMulticast, reliable, withvalidation)
		void NetMulticast_BroadcastControlPointsScore(FZanshinScoreInfo Info);
	bool NetMulticast_BroadcastControlPointsScore_Validate(FZanshinScoreInfo Info);
	void NetMulticast_BroadcastControlPointsScore_Implementation(FZanshinScoreInfo Info);

	FZanshinScoreInfo CurrentScoreInfo;

	/** UI. Broadcasted Control Points Score Info to all Clients. */
	UFUNCTION(BlueprintImplementableEvent, meta = (DisplayName = "Update_ControlPointScores"))
		void BP_Event_UpdateControlPointScores(FZanshinScoreInfo Info);

	//////////////////////////////////////////////////////////////////////////
	//	Other

	/** [Server] Reset Elapsed Time */
	UFUNCTION(BlueprintCallable, Category=Timer)
	void ResetElapsedTime();

	/** [Server] Register a player in their team. */
	void HandlePlayerSpawn(AZanshinPlayerState* NewState, int32 NewTeamNumber);

	void RequestFinishAndExitToMainMenu();

	/** [Server] Returns the score of the blue team. */
	UFUNCTION(BlueprintCallable, Category = UI)
	int32 GetBlueScore();

	/** [Server] Returns the score of the red team. */
	UFUNCTION(BlueprintCallable, Category = UI)
	int32 GetRedScore();

	/** [Server] Adds Score to a team. */
	void AddTeamScore(int32 AddedScore, int32 TeamNum);

	/** [Client] Used in clients to show the blue team's info on the score screen. */
	UFUNCTION(BlueprintCallable, Category = UI)
	TArray<FZanshinPlayerNetInfo> GetBluePlayersInfo();

	/** [Client] Used in clients to show the red team's info on the score screen. */
	UFUNCTION(BlueprintCallable, Category = UI)
	TArray<FZanshinPlayerNetInfo> GetRedPlayersInfo();

	/** Amount of time elapsed. */
	int32 GameTime;

	/** Called when ElapsedTime changes. */
	virtual void OnRep_ElapsedTime() override;

	/** Called To Display the EndGame HUD. BP.
	*	TODO: Handle it being already displayed. Add End of Match Menu. 
	*/
	UFUNCTION(BlueprintImplementableEvent, meta = (DisplayName = "ShowEndOfMatchHUD"))
	void BP_Event_ShowEndOfMatch();

	/** Populate the TeamInfo struct. */
	void GetTeamInfo(FZanshinTeam& InTeam);

	/** Populate the TeamInfo struct. */
	void BroadcastControlPointScore(FZanshinTeam ScoringTeam);

	UFUNCTION(BlueprintImplementableEvent, meta = (DisplayName = "HideLoadingScreen"))
	void BP_Event_HideLoadingScreenAfterSeconds();

	/** Check for time in the game. */
	void CheckGameStatus();
	void ShowEndScreen();

	/** Receive ScoreInfo from the GameMode. */
	void BroadcastScores(FZanshinScoreInfo ScoreInfo);

	/** Tells each PlayerState to save it's  */
	void HandleEndOfGame();

	void HandleAllPlayersJoined();
	void CheckPlayerJoined();
	void StartMatchTimer();

	//////////////////////////////////////////////////////////////////////////
	//	Networking

	bool bGameEndHandled = false;

	/** Provides a ScoreInfo from the server to the UI. */
	UFUNCTION(NetMulticast, reliable, withvalidation)
	void NetMulticast_UpdateScore(FZanshinScoreInfo ScoreInfo);
	void NetMulticast_UpdateScore_Implementation(FZanshinScoreInfo ScoreInfo);
	bool NetMulticast_UpdateScore_Validate(FZanshinScoreInfo ScoreInfo);

	/** Tell
	*/
	UFUNCTION(NetMulticast, reliable, withvalidation)
	void NetMulticast_StartMatchTimer();
	void NetMulticast_StartMatchTimer_Implementation();
	bool NetMulticast_StartMatchTimer_Validate();

	/** Shows the Screen at the end of the game. */
	UFUNCTION(NetMulticast, unreliable, withvalidation)
	void NetMulticast_ShowEndHud();
	void NetMulticast_ShowEndHud_Implementation();
	bool NetMulticast_ShowEndHud_Validate();

};