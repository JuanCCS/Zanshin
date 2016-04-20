// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "Engine/GameInstance.h"
#include "Player/ZanshinPlayerState.h"
#include "ZanshinGameInstance.generated.h"

USTRUCT(BlueprintType)
struct FPlayerEndOfMatchInfo
{
	GENERATED_USTRUCT_BODY()

		FPlayerEndOfMatchInfo(){
		PlayerName = "";
		PlayerKills = 0;
		PlayerDeaths = 0;
		PlayerAssists = 0;
		PlayerScore = 0;
		PlayerTeam = 0;
	}

	/** Player chosen username */
	UPROPERTY(BlueprintReadWrite)
		FString PlayerName;

	/** Player's Kills */
	UPROPERTY(BlueprintReadWrite)
		int32 PlayerKills;

	/** Player's Deaths */
	UPROPERTY(BlueprintReadWrite)
		int32 PlayerDeaths;

	/** Player's Deaths */
	UPROPERTY(BlueprintReadWrite)
		int32 PlayerAssists;

	/** Player's Current Score */
	UPROPERTY(BlueprintReadWrite)
		int32 PlayerScore;

	UPROPERTY(BlueprintReadWrite)
		int32 PlayerTeam;
};

USTRUCT(BlueprintType)
struct FLocalPlayerEndOfMatchInfo
{
	GENERATED_USTRUCT_BODY()

		FLocalPlayerEndOfMatchInfo(){
		PlayerName = "";
		PlayerAccuracy = 0.f;
		MostKilledPlayer = "";
		MostDeathsByPlayer = "";
	}

	/** Player chosen username */
	UPROPERTY(BlueprintReadWrite)
		FString PlayerName;

	/** Player's Deaths */
	UPROPERTY(BlueprintReadWrite)
		float PlayerAccuracy;

	/** Player's Current Score */
	UPROPERTY(BlueprintReadWrite)
		FString MostKilledPlayer;

	/** Player's Current Score */
	UPROPERTY(BlueprintReadWrite)
		FString MostDeathsByPlayer;

	/** Hits, etc. */
};

USTRUCT(BlueprintType)
struct FZanshinEndOfMatchInfo
{
	GENERATED_USTRUCT_BODY()

		FZanshinEndOfMatchInfo(){
	}

	/** Player chosen username */
	UPROPERTY(BlueprintReadWrite)
	FString PlayerName;

	/** Player's Kills */
	UPROPERTY(BlueprintReadWrite)
	int32 PlayerKills;

	/** Player's Deaths */
	UPROPERTY(BlueprintReadWrite)
	int32 PlayerDeaths;

	/** Player's Assists */
	UPROPERTY(BlueprintReadWrite)
	int32 PlayerAssists;

	/** Player's Current Score */
	UPROPERTY(BlueprintReadWrite)
	int32 PlayerScore;
};

/**
 * 
 */
UCLASS()
class ZANSHIN_API UZanshinGameInstance : public UGameInstance
{
	GENERATED_BODY()

	const FString GAME_NAME = "Zanshin";
	const FString BUILD_VERSION = "Version_F";
public: 

	UZanshinGameInstance(const FObjectInitializer& ObjectInitializer);

	~UZanshinGameInstance();

	const int32 MAX_PLAYERS = 32;

	FName ZanshinName = FName(*GAME_NAME);

	FName BuildVersion = FName(*BUILD_VERSION);

	FName CurrentSessionName;

	/////////////////////////////////////////////////////////////////////////
	/////	LOBBY

	/** Player's team number. Chosen in Lobby. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PlayerInfo")
	int32 PlayerTeamNumber;

	/** Player's name. Chosen in Lobby. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PlayerInfo")
	FName PlayerName = FName(TEXT("DefaultUsername"));

	/** Triggers a BP Event. . */
	UFUNCTION(BlueprintImplementableEvent, meta = (DisplayName = "JoinedGame"))
	void OnClientJoinGame();

	/** BP Function. Called when a client joins the lobby. */
	UFUNCTION(BlueprintCallable, Category = "HUD")
	void OnClientJoinLobby();

	/** Add a Player Widget To the List. (Blueprints) */
	UFUNCTION(BlueprintImplementableEvent, meta = (DisplayName = "AddPlayerWidget"))
	void AddPlayerWidget(FName NetUserName, bool NetIsReady, int32 NetTeamNumber);

	//////////////////////////////////////////////////////////////////////////
	/////	FIND GAME.

	/** Server List widget. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "WIDGETS")
	TSubclassOf<class UUserWidget> wServerList;

	/** Server Row widget. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "WIDGETS")
	TSubclassOf<class UUserWidget> wServerRow;

	/** Add a row to the server list. (Blueprints) */
	UFUNCTION(BlueprintImplementableEvent, meta = (DisplayName = "AddRowToServerList"))
	void AddRowToServerList(const FString& ServerName, int32 Players);

	/** Creates a server row for each search result. Each row allows the player to join a different session. */
	void AddServerRows(TArray<FOnlineSessionSearchResult> results);

	//////////////////////////////////////////////////////////////////////////
	/////	UI

	/** Current Widget variable. Used along with the ShowUserWidget method to handle the UI. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Widgets")
	UUserWidget* CurrentWidget;
	
	UUserWidget* MyServerList;
	UUserWidget* MyServerRow;

	APlayerController* MyController;

	/** Displays a widget class in the UI. */
	UFUNCTION(BlueprintCallable, Category = "HUD")
	void ShowUserWidget(TSubclassOf<class UUserWidget> wDesiredWidget, bool bIsInputModeUIOnly);

	//////////////////////////////////////////////////////////////////////////
	/////	SESSIONS

	/** Host a Game. Called from the Main Menu. */
	UFUNCTION(BlueprintCallable, Category = "HUD")
	void HostGameEvent(FString GameName);

	/** Find a Game. Called from the Main Menu. */
	UFUNCTION(BlueprintCallable, Category = "HUD")
	void FindGameEvent();

	/** Host a Game. Called from within the Server List UI. */
	UFUNCTION(BlueprintCallable, Category = "HUD")
	void JoinGameEvent(FString SessionName);

	/** Must be called at the end of each game. */
	UFUNCTION(BlueprintCallable, Category = "Network|Test")
	void DestroySessionEvent();		

	/** Must be called at the end of each game. */
	UFUNCTION(BlueprintCallable, Category = "Network|Test")
	void EndSessionEvent();

	void CleanupOnlineSessionsOnReturnToMainMenu();

	/** Must be called at the end of each game. */
	UFUNCTION(BlueprintCallable, Category = "HUD")
	void HideSessionEvent();

	/** Must be called at the end of each game. */
	UFUNCTION(BlueprintCallable, Category = "HUD")
	void ShowSessionEvent();

	/**
	*	Function to host a game!
	*
	*	@Param		UserID			User that started the request
	*	@Param		SessionName		Name of the Session
	*	@Param		bIsLAN			Is this a LAN Game?
	*	@Param		bIsPresence		"Is the Session to create a presence Session"
	*	@Param		MaxNumPlayers	Number of Maximum allowed players on this "Session" (Server)
	*/
	bool HostSession(TSharedPtr<const FUniqueNetId> UserId, FName SessionName, bool bIsLAN, bool bIsPresence, int32 MaxNumPlayers, FString GameName);

	/* Delegate called when session created */
	FOnCreateSessionCompleteDelegate OnCreateSessionCompleteDelegate;
	/* Delegate called when session started */
	FOnStartSessionCompleteDelegate OnStartSessionCompleteDelegate;

	/** Handles to registered delegates for creating/starting a session */
	FDelegateHandle OnCreateSessionCompleteDelegateHandle;
	FDelegateHandle OnStartSessionCompleteDelegateHandle;

	

	/* Data structure that holds the session information. */
	TSharedPtr<class FOnlineSessionSettings> SessionSettings;
	
	/** ENDING SESSIONS */
	FOnEndSessionCompleteDelegate OnEndSessionCompleteDelegate;
	FDelegateHandle OnEndSessionCompleteDelegateHandle;
	virtual void OnEndSessionComplete(FName SessionName, bool bWasSuccessful);

	/**
	*	Function fired when a session create request has completed
	*
	*	@param SessionName the name of the session this callback is for
	*	@param bWasSuccessful true if the async action completed without error, false if there was an error
	*/
	virtual void OnCreateSessionComplete(FName SessionName, bool bWasSuccessful);

	/**
	*	Function fired when a session start request has completed
	*
	*	@param SessionName the name of the session this callback is for
	*	@param bWasSuccessful true if the async action completed without error, false if there was an error
	*/
	void OnStartOnlineGameComplete(FName SessionName, bool bWasSuccessful);

	/**
	*	Find an online session
	*
	*	@param UserId user that initiated the request
	*	@param SessionName name of session this search will generate
	*	@param bIsLAN are we searching LAN matches
	*	@param bIsPresence are we searching presence sessions
	*/
	void FindSessions(TSharedPtr<const FUniqueNetId> UserId, FName SessionName, bool bIsLAN, bool bIsPresence);

	/** Delegate for searching for sessions */
	FOnFindSessionsCompleteDelegate OnFindSessionsCompleteDelegate;

	/** Handle to registered delegate for searching a session */
	FDelegateHandle OnFindSessionsCompleteDelegateHandle;

	TSharedPtr<class FOnlineSessionSearch> SessionSearch;

	TArray<FOnlineSessionSearchResult> SearchResults;

	/**
	*	Get a search result given it's name.
	*
	*	@param SessionName		Name of the Session user wants to join.
	*
	*	@return FOnlineSessionSearchResult The result with name SessionName,
	*	if none found return first in array.
	*/
	FOnlineSessionSearchResult GetResultWithName(FString SessionName);

	/**
	*	Delegate fired when a session search query has completed
	*
	*	@param bWasSuccessful true if the async action completed without error, false if there was an error
	*/
	void OnFindSessionsComplete(bool bWasSuccessful);

	/**
	*	Joins a session via a search result
	*
	*	@param SessionName name of session
	*	@param SearchResult Session to join
	*
	*	@return bool true if successful, false otherwise
	*/
	bool ZJoinSession(TSharedPtr<const FUniqueNetId> UserId, FName SessionName, const FOnlineSessionSearchResult& SearchResult);

	/** Delegate for joining a session */
	FOnJoinSessionCompleteDelegate OnJoinSessionCompleteDelegate;

	/** Handle to registered delegate for joining a session */
	FDelegateHandle OnJoinSessionCompleteDelegateHandle;

	/**
	*	Delegate fired when a session join request has completed
	*
	*	@param SessionName the name of the session this callback is for
	*	@param bWasSuccessful true if the async action completed without error, false if there was an error
	*/
	void OnJoinSessionComplete(FName SessionName, EOnJoinSessionCompleteResult::Type Result);

	/** Delegate for destroying a session */
	FOnDestroySessionCompleteDelegate OnDestroySessionCompleteDelegate;

	/** Handle to registered delegate for destroying a session */
	FDelegateHandle OnDestroySessionCompleteDelegateHandle;

	/**
	*	Delegate fired when a destroying an online session has completed
	*
	*	@param SessionName the name of the session this callback is for
	*	@param bWasSuccessful true if the async action completed without error, false if there was an error
	*/
	virtual void OnDestroySessionComplete(FName SessionName, bool bWasSuccessful);

	/** [Server] Player's Unique Id and Team. TODO: Turn into struct. */
	TMap<FUniqueNetIdRepl, int32>PlayerTeams;

	/** [Server] Player's Unique Id and Names. TODO: Turn into struct. */
	TMap<FUniqueNetIdRepl, FName>PlayerNames;

	/** [Server] Register a player. Before traveling because playerstates are nuked. 
	*
	*	@param ZPlayerState		The Player's PlayerState.
	*	@param Team				Player chosen Team.
	*/
	void RegisterPlayer(AZanshinPlayerState* ZPlayerState, int32 Team);

	/** [Server] Register a player's name. Before traveling because playerstates are nuked.
	*
	*	@param ZPlayerState		The Player's PlayerState.
	*	@param Team				Player chosen Name.
	*/
	UFUNCTION(BlueprintCallable, category = "Player Registration")
	void RegisterPlayerName(AZanshinPlayerState* ZPlayerState, FName Name);

	////////////////////////////////////////////////////////////////////
	// PLAYER STATS

	/** [Client] The local player's info regarding kills, accuracy. For End of Game. */
	FPlayerStatsData LocalPlayerData;

	/** [Client] Return LocalPlayerData */
	UFUNCTION(BlueprintCallable, category = "End Screen")
	FPlayerStatsData GetData();

	/** [Client] Populate LocalPlayerData */
	void SetPlayerData(FPlayerStatsData Stats);

	/** [Client] The info of the Red Team. + Getter & Setter.  */
	TArray<FZanshinEndOfMatchInfo> RedTeam;

	UFUNCTION(BlueprintCallable, category = "End Screen")
	TArray<FZanshinEndOfMatchInfo> GetRedTeam();

	UPROPERTY(BlueprintReadWrite, category = "End Screen")
	int32 RedTeamControlPointScore;

	void SetRedTeam(FZanshinTeam InRedTeam);

	/** [Client] The info of the Red Team.  + Getter & Setter. */
	TArray<FZanshinEndOfMatchInfo> BlueTeam;

	UPROPERTY(BlueprintReadWrite, category = "End Screen")
	int32 BlueTeamControlPointScore;

	UFUNCTION(BlueprintCallable, category = "End Screen")
	TArray<FZanshinEndOfMatchInfo> GetBlueTeam();

	void SetBlueTeam(FZanshinTeam InBlueTeam);

};
