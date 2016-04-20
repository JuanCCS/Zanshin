// Fill out your copyright notice in the Description page of Project Settings.

#pragma once
#include "GameFramework/PlayerController.h"
#include "Networking/NetworkDebugInterface.h"
#include "GameMode/ZanshinGameInstance.h"
#include "ZanshinSpectatorPawn.h"
#include "ZanshinPlayerController.generated.h"

/**
*
*/

/** Kill Types depending on the body part hit or attack type. */
UENUM(BlueprintType)
enum class ChatRecipients : uint8
{
	/** Default value. */
	NONE 			UMETA(DisplayName = "NONE"),

	/** Player was killed via Melee Attack. */
	RED_TEAM 			UMETA(DisplayName = "RED_TEAM"),

	/** Player was killed via HeadShot. */
	BLUE_TEAM		UMETA(DisplayName = "BLUE_TEAM"),

	/** Player was killed via explosions or arrows shot to the body. */
	ALL_PLAYERS			UMETA(DisplayName = "ALL_PLAYERS")
};

USTRUCT(BlueprintType)
struct FZanshinChatMsg
{
	GENERATED_USTRUCT_BODY()

	FZanshinChatMsg(){
		Recipient = ChatRecipients::NONE;
		Sender = "";
		Content = "";
	}

	/** Intended recipients of the message (Type of message). */
	UPROPERTY(BlueprintReadWrite)
	ChatRecipients Recipient;

	/** Person who sends the message. */
	UPROPERTY(BlueprintReadWrite)
	FString Sender;

	/** Contents of the message. */
	UPROPERTY(BlueprintReadWrite)
	FString Content;

};

UCLASS(config = Game)
class ZANSHIN_API AZanshinPlayerController : public APlayerController, public INetworkDebugInterface
{

	GENERATED_BODY()

public:

	AZanshinPlayerController();

	/** stores pawn location at last player death, used where player scores a kill after they died **/
	FVector LastDeathLocation;

	/** after all game elements are created */
	virtual void PostInitializeComponents() override;

	virtual void BeginPlay() override;
	virtual void SetupInputComponent() override;
	virtual void TickActor(float DeltaTime, enum ELevelTick TickType, FActorTickFunction& ThisTickFunction) override;
	//End AActor interface

	/** respawn after dying */
	virtual void UnFreeze() override;

	/** update camera when pawn dies */
	virtual void PawnPendingDestroy(APawn* P) override;

	////////////////////////////////////////////
	/// Camera

	/** Store the spots for the spectator after death. */
	TArray<ACameraActor*> CameraSpots;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, replicated, Category = Gameplay)
	int32 TimeToSwitchToSpectatorCamera;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, replicated, Category = Gameplay)
	bool bGoKillerCamera;

	/** sets spectator location and rotation */
	UFUNCTION(reliable, client)
	void ClientSetSpectatorCamera(FVector CameraLocation, FRotator CameraRotation);

	/** sets spectator location and rotation for the map camera */
	void ClientSetSpectatorCameraForMap();

	/** try to find spot for death cam */
	bool FindDeathCameraSpot(FVector& CameraLocation, FRotator& CameraRotation);

	UFUNCTION(reliable, client)
	void ClientGoToKillerCamera();

	UFUNCTION(BlueprintImplementableEvent, meta = (DisplayName = "GoKillerCamera"))
	void BP_Event_GoKillerCamera(AZanshinCharacter* Enemy);

	UFUNCTION(BlueprintImplementableEvent, meta = (DisplayName = "RemoveHUDDeathCamera"))
	void BP_Event_RemoveHUDDeathCamera();

	UFUNCTION(BlueprintCallable, Category = Gameplay)
	void EndGameCamera();

	//////////////////////////////////////////////////////////////////////////
	// Score

	UFUNCTION(BlueprintCallable, Category = "Score")
	void DisplayScore();

	UFUNCTION(BlueprintCallable, Category = "Score")
	void HideScore();

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Score")
	bool bIsDisplayingScore;

	////////////////////////////////////////////
	/// Team

	/** The team for each character **/
	UPROPERTY(EditAnywhere, BlueprintReadWrite, replicated, Category = Gameplay)
	int32 CharacterTeam = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, replicated, Category = Gameplay)
	FName PlayerName = "DefaultName";

	////////////////////////////////////////////
	/// Enemy

	UPROPERTY(EditAnywhere, BlueprintReadOnly, replicated, Category = Gameplay)
	class AZanshinCharacter* Enemy;

	void KilledBy(class AZanshinCharacter* Enemy);

	////////////////////////////////////////////
	/// Spectator

	UPROPERTY(EditAnywhere, BlueprintReadWrite, replicated, Category = Gameplay)
	TSubclassOf<class AZanshinSpectatorPawn> SpectatorClass;

	class AZanshinSpectatorPawn* SpectatorPawn;

	UPROPERTY(EditAnywhere, replicated, Category = Gameplay)
	bool bIsSpectator;
	
	void Spectator();
	void ActivateSpectatorPawn();

private:

	bool bIsEndGame;

	FVector PawnLocationSpectator;

	/** Timer to control when the camera moves form spectating the player die to to watch the map. */
	FTimerHandle TimerHandle_ChangeToSpectatorMapCamera;

public:

	////////////////////////////////////////////
	/// NETWORKING

	/** Check if Player is Ready. */
	bool CheckReadyState();

	/** Create Player Widgets on Lobby */
	void CreatePlayerWidgets(AZanshinPlayerState* InPlayerState);

	/** Handle when a player has joined the lobby. */
	void OnJoinedLobby(APlayerState* InPlayerState, FName InPlayerName);

	/** Tell server that a player joined the lobby. */
	UFUNCTION(reliable, server, WithValidation)
		void ServerOnJoinedLobby(APlayerState* InPlayerState, FName InPlayerName);
	virtual void ServerOnJoinedLobby_Implementation(APlayerState* InPlayerState, FName InPlayerName);
	virtual bool ServerOnJoinedLobby_Validate(APlayerState* InPlayerState, FName InPlayerName);

	/** Set up the information. */
	void SetupInfo();

	/** Set up info on the client. */
	UFUNCTION(reliable, client)
	void Client_SetupInfo();
	virtual void Client_SetupInfo_Implementation();

	/** Register Player on the Server. */
	UFUNCTION(reliable, server, WithValidation)
	void Server_SetupInfo(FName InPlayerName, int32 PlayerTeam);
	virtual void Server_SetupInfo_Implementation(FName InPlayerName, int32 PlayerTeam);
	virtual bool Server_SetupInfo_Validate(FName InPlayerName, int32 PlayerTeam);

	/**  [Server] Called when all players are ready. Executes console command on BP. */
	UFUNCTION(BlueprintImplementableEvent, meta = (DisplayName = "DoServerTravel"))
	void DoServerTravel();

	/** Change to Spectator Camera. TODO: Fix meshes visibility and actor rotation. */
	UFUNCTION(server, reliable, withvalidation)
	void Server_SetSpectatorCamera(FVector CameraLocation, FRotator CameraRotation);
	bool Server_SetSpectatorCamera_Validate(FVector CameraLocation, FRotator CameraRotation);
	void Server_SetSpectatorCamera_Implementation(FVector CameraLocation, FRotator CameraRotation);

	/** Find a Spectator Camera. **/
	UFUNCTION(server, reliable, withvalidation)
	void Server_SetSpectatorCameraForMap();
	bool Server_SetSpectatorCameraForMap_Validate();
	void Server_SetSpectatorCameraForMap_Implementation();

	/** Find a Death Camera Spot. **/
	UFUNCTION(server, reliable, withvalidation)
	void Server_FindDeathCameraSpot(FVector CameraLocation, FRotator CameraRotation);
	bool Server_FindDeathCameraSpot_Validate(FVector CameraLocation, FRotator CameraRotation);
	void Server_FindDeathCameraSpot_Implementation(FVector CameraLocation, FRotator CameraRotation);

	/** Go to the killer camera after die. */
	UFUNCTION(server, reliable, withvalidation)
	void Server_GoKillerCamera();
	bool Server_GoKillerCamera_Validate();
	void Server_GoKillerCamera_Implementation();

	/** Get the team number. **/
	void GetTeamNumber();

	/** Register the player. **/
	void RegisterPlayer(int32 InTeamNumber, FName InPlayerName);

	/** Find all characters and make their mesh visible. */
	void UpdateMeshVisibility(bool bSeeFirstPerson);

	UFUNCTION(Client, Reliable, withvalidation)
	void Client_UpdateMeshVisibility();
	bool Client_UpdateMeshVisibility_Validate();
	void Client_UpdateMeshVisibility_Implementation();

	UFUNCTION(server, reliable, withvalidation)
	void Server_SetJoinedMap();
	bool Server_SetJoinedMap_Validate();
	void Server_SetJoinedMap_Implementation();

	UFUNCTION(Client, Reliable, withvalidation)
	void Client_ShowEndMatchLoadingScreen(int32 WinningTeam);
	bool Client_ShowEndMatchLoadingScreen_Validate(int32 WinningTeam);
	void Client_ShowEndMatchLoadingScreen_Implementation(int32 WinningTeam);

	UFUNCTION(BlueprintImplementableEvent, meta = (DisplayName = "ShowEndOfMatchLoadingScreen"))
	void BP_Event_ShowEndOfMatchLoadingScreen(int32 WinningTeam);

	UFUNCTION(BlueprintCallable, category = CHAT)
	FZanshinChatMsg MakeChat(ChatRecipients MessageType, FString Sender, FString Content);

	UFUNCTION(server, reliable, withvalidation)
	void Server_SetSpectator();
	bool Server_SetSpectator_Validate();
	void Server_SetSpectator_Implementation();
};