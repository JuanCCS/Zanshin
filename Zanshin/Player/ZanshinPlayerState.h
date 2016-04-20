// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "GameFramework/PlayerState.h"
#include "Networking/NetworkDebugInterface.h"
#include "ZanshinPlayerState.generated.h"

UENUM(BlueprintType)
enum class TeamColorProperty : uint8
{
	/** Default value. */
	CP_None 	UMETA(DisplayName = "None"),
	/** Red Team. (1) */
	CP_Red 		UMETA(DisplayName = "Red"),
	/** Blue Team. (2) */
	CP_Blue		UMETA(DisplayName = "Blue")
};

/* Struct used to hold the players' score and team information. */
USTRUCT(BlueprintType)
struct FZanshinTeam
{
	GENERATED_USTRUCT_BODY()

	/** All the players in the team. */
	UPROPERTY(BlueprintReadOnly, Category = Team)
	TArray<APlayerState*> TeamPlayers;

	/** Color of the team. */
	UPROPERTY(BlueprintReadOnly, Category = Team)
	TeamColorProperty CurrentColor = TeamColorProperty::CP_None;

	/**
	* Register a player in the team.
	*
	* @param	Player	The player you want to register.
	*/
	void RegisterPlayer(APlayerState* Player){

		TeamPlayers.AddUnique(Player);

	}

	void Validate(){
		for (APlayerState* Player : TeamPlayers){
			//check(Player);
			if (Player == nullptr){
				TeamPlayers.Remove(Player);
			}
		}
	}

	/** [Server] Score for control points and special events. */
	UPROPERTY(BlueprintReadOnly, Category = Team)
	int32 NonKillRelatedScore;

	/**
	*  [Server] Add non-kill related score. e.g. Control Points.
	*
	* @param	Score	The amount you want to add to the Non Kill Related Score.
	*/
	void AddScore(int32 Score){

		NonKillRelatedScore += Score;
		GetScore();
	}

	/** [Server] Returns the score of the team. */
	UPROPERTY(BlueprintReadOnly, Category = Team)
	int32 TeamScore = 0;

	int32 GetScore(){
		TeamScore = 0;
		for (APlayerState* PlayerData : TeamPlayers)
		{
			TeamScore += PlayerData->Score;
		}

		TeamScore += NonKillRelatedScore;

		return TeamScore;
	}

	FZanshinTeam()
	{
		NonKillRelatedScore = 0;
	}
};

USTRUCT(BlueprintType)
struct FPlayerStatsData
{
	GENERATED_USTRUCT_BODY()

	/** The number of arrows fired. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = Gameplay)
	int32 ArrowsFired;

	/** The number of arrows that hit a enemy. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = Gameplay)
	int32 ArrowsThatHit;

	/** The number of hit in the head. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = Gameplay)
	int32 Head;

	/** The number of hit in the chest. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = Gameplay)
	int32 Chest;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = Gameplay)
	int32 R_Arm;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = Gameplay)
	int32 L_Arm;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = Gameplay)
	int32 R_Leg;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = Gameplay)
	int32 L_Leg;

	float GetAccuracy() { return (ArrowsThatHit * 100) / ArrowsFired; }
	
	void UpdateBodyPartHit(FName BoneName)
	{
		if (BoneName == "head") {
			Head++;
		} else if (BoneName == "spine_03" || BoneName == "spine_02" || BoneName == "spine_01") {
			Chest++;
		} else if (BoneName == "upperarm_r" || BoneName == "lowerarm_r" || BoneName == "hand_r") {
			R_Arm++;
		} else if (BoneName == "upperarm_l" || BoneName == "lowerarm_l" || BoneName == "hand_l") {
			L_Arm++;
		} else if (BoneName == "thigh_r" || BoneName == "calf_r" || BoneName == "foot_r") {
			R_Leg++;
		} else if (BoneName == "thigh_l" || BoneName == "calf_l" || BoneName == "foot_l") {
			L_Leg++;
		}
	}

	/** Defaults. */
	FPlayerStatsData()
	{
		ArrowsFired = 0;
		ArrowsThatHit = 0;
		Head = 0;
		Chest = 0;
		R_Arm = 0;
		L_Arm = 0;
		R_Leg = 0;
		L_Leg = 0;
	}
};

/**
 *
 */
UCLASS()
class ZANSHIN_API AZanshinPlayerState : public APlayerState, public INetworkDebugInterface
{
	GENERATED_BODY()

public:

	UPROPERTY(replicated, BlueprintReadWrite, Category = PlayerState)
	bool bPlayerIsInMap = false;

	UFUNCTION(BlueprintCallable, Category = LoadMap)
	void NotifyJoinedMap();

	UPROPERTY(replicated, BlueprintReadWrite, Category = PlayerState)
	bool bPlayerIsReady;

	UPROPERTY(EditAnywhere, replicated, BlueprintReadWrite, Category = PlayerState)
	bool bPlayerScoredKill;

	/** Player's current score. */
	UPROPERTY(replicatedUsing = OnRep_KillCount, BlueprintReadOnly, Category = PlayerState)
	int32 KillCount;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Score")
	int32 KillStreakActive = 3;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Score")
	int32 KillStreakLimit = 9;

	UPROPERTY(EditAnywhere, Replicated, BlueprintReadWrite, Category = "Score")
	int32 CurrentKillStreak = 0;

	/** Players`s kill assists. */
	UPROPERTY(replicatedUsing = OnRep_KillCount, BlueprintReadOnly, Category = PlayerState)
	int32 KillAssist;

	/** number of deaths */
	UPROPERTY(BlueprintReadWrite, Transient, Replicated)
	int32 NumDeaths;

	UPROPERTY(replicated, BlueprintReadWrite, Category = PlayerState)
	FName UserName = FName("TestUsername");

	/** Player's current score. */
	UPROPERTY(replicatedUsing = OnRep_TeamNumber, BlueprintReadWrite, Category = PlayerState)
	int32 TeamNumber = 0;

	UPROPERTY(replicatedUsing = OnRep_IsReadyLobby, BlueprintReadWrite, Category = PlayerState)
	bool bIsReadyLobby = false;

	void PlayerKill(FString KilledPlayerName, int32 KillValue);

	UFUNCTION(BlueprintImplementableEvent, meta = (DisplayName = "KillStreakActiveEvent"))
	void BP_Event_KillstreakActive(int32 KillstreakCount);

	UFUNCTION()
	void OnRep_KillCount();

	UFUNCTION(BlueprintImplementableEvent, meta = (DisplayName = "OnRep_TeamNumber"))
	void OnRep_TeamNumber();

	UFUNCTION()
	void OnRep_IsReadyLobby();

	////////////////////////////////////////////////////////////////////////////////
	//// Score, Kill, Death

	/** player killed someone */
	void ScoreKill(AZanshinPlayerState* Victim, int32 Points);

	void ScoreAssist(int32 Points);

	/** player killed someone */
	UFUNCTION(BlueprintCallable, category = PlayerState)
	void ResetScoreKill();

	/** player died */
	void ScoreDeath(AZanshinPlayerState* KilledBy, int32 Points);

	/** Sends kill (excluding self) to clients */
	UFUNCTION(Reliable, Client)
	void InformAboutKill(class AZanshinPlayerState* KillerPlayerState, const UDamageType* KillerDamageType, class AZanshinPlayerState* KilledPlayerState);

	/** broadcast death to local clients */
	UFUNCTION(Reliable, NetMulticast)
	void BroadcastDeath(class AZanshinPlayerState* KillerPlayerState, const UDamageType* KillerDamageType, class AZanshinPlayerState* KilledPlayerState);

	UPROPERTY(replicated, BlueprintReadWrite)
	FPlayerStatsData ZanshinPlayerData;

	void SavePlayerData();

	void SaveTeamData(FZanshinTeam RedTeam, FZanshinTeam BlueTeam);

	/** Save each player's stats on their game instance. */
	UFUNCTION(Client, Reliable)
	void Client_SavePlayerData();
	void Client_SavePlayerData_Implementation();
	bool Client_SavePlayerData_Validate();

	/** Save each player's stats on their game instance. */
	UFUNCTION(Client, Reliable)
	void Client_SaveTeamData(FZanshinTeam RedTeam, FZanshinTeam BlueTeam);
	void Client_SaveTeamData_Implementation(FZanshinTeam RedTeam, FZanshinTeam BlueTeam);
	bool Client_SaveTeamData_Validate(FZanshinTeam RedTeam, FZanshinTeam BlueTeam);

	////////////////////////////////////////////////////////////////////////////////
	//// NETWORKING

	/** Change team number. */
	UFUNCTION(reliable, server, WithValidation)
	void ServerSetTeamNumber(int32 NewTeamNumber);
	virtual void ServerSetTeamNumber_Implementation(int32 NewTeamNumber);
	virtual bool ServerSetTeamNumber_Validate(int32 NewTeamNumber);

	/** Set Username. */
	UFUNCTION(BlueprintCallable, reliable, server, WithValidation, Category = PlayerState)
	void ServerSetUserName(FName NewUserName);
	virtual void ServerSetUserName_Implementation(FName NewUserName);
	virtual bool ServerSetUserName_Validate(FName NewUserName);

	/** Add one widget for each player. */
	UFUNCTION(Client, Reliable)
	void Client_AddPlayerWidget(FName NetUserName, bool NetIsReady, int32 NetTeamNumber);
	virtual void Client_AddPlayerWidget_Implementation(FName NetUserName, bool NetIsReady, int32 NetTeamNumber);
	virtual bool Client_AddPlayerWidget_Validate(FName NetUserName, bool NetIsReady, int32 NetTeamNumber);

	/** When player clicks ready button. */
	UFUNCTION(BlueprintCallable, reliable, server, WithValidation, Category = PlayerState)
	void ServerSetReady(bool ReadyState);
	virtual void ServerSetReady_Implementation(bool ReadyState);
	virtual bool ServerSetReady_Validate(bool ReadyState);

	/** Score points. */
	UFUNCTION(BlueprintCallable, reliable, server, WithValidation, Category = Score)
	void ServerScorePoints(int32 Points);
	virtual void ServerScorePoints_Implementation(int32 Points);
	virtual bool ServerScorePoints_Validate(int32 Points);

	/** Killstreak active. */
	UFUNCTION(BlueprintCallable, reliable, server, WithValidation, Category = Score)
		void Server_KillstreakActive(int32 KillstreakCount);
	virtual void Server_KillstreakActive_Implementation(int32 KillstreakCount);
	virtual bool Server_KillstreakActive_Validate(int32 KillstreakCount);

	UFUNCTION(reliable, client, WithValidation, Category = Score)
		void Client_KillstreakActive(int32 KillstreakCount);
	virtual void Client_KillstreakActive_Implementation(int32 KillstreakCount);
	virtual bool Client_KillstreakActive_Validate(int32 KillstreakCount);

	UFUNCTION(Blueprintcallable, category = teams)
	void SetTeamNumberByBP(int32 InTeamNumber);

	void AddPlayerWidget(FName NetUserName, bool NetIsReady, int32 NetTeamNumber);

	int32 GetTeamNumber();

	UFUNCTION(reliable, server, WithValidation)
	void Server_GetTeamNumber();
	virtual void Server_GetTeamNumber_Implementation();
	virtual bool Server_GetTeamNumber_Validate();

	UFUNCTION(reliable, server, withvalidation)
	void Server_ResetScoreKill();
	virtual void Server_ResetScoreKill_Implementation();
	virtual bool Server_ResetScoreKill_Validate();

	void PlayKillStreakSound(int32 KillStreakNumber);

	UFUNCTION(reliable, client, withvalidation)
	void Client_PlayKillStreakSound(int32 KillStreakNumber);
	bool Client_PlayKillStreakSound_Validate(int32 KillStreakNumber);
	void Client_PlayKillStreakSound_Implementation(int32 KillStreakNumber);

	UFUNCTION(reliable, client, withvalidation)
		void Client_ProvideAsisstFeedback();
	bool Client_ProvideAsisstFeedback_Validate();
	void Client_ProvideAsisstFeedback_Implementation();
	
	UFUNCTION(BlueprintImplementableEvent, meta = (displayname = "Assist Widget"))
	void BP_Event_ShowAssistFeedback();

protected:

	/** helper for scoring points */
	void ScorePoints(int32 Points);

	void KillstreakActive(int32 KillstreakCount);
};