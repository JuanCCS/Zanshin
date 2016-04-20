// Fill out your copyright notice in the Description page of Project Settings.

#include "Zanshin.h"
#include "Player/ZanshinPlayerController.h"
#include "ZanshinGameInstance.h"
#include "ZanshinGameState.h"

UZanshinGameInstance::UZanshinGameInstance(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer){

	/** Bind function for CREATING a Session */
	OnCreateSessionCompleteDelegate = FOnCreateSessionCompleteDelegate::CreateUObject(this, &UZanshinGameInstance::OnCreateSessionComplete);
	OnStartSessionCompleteDelegate = FOnStartSessionCompleteDelegate::CreateUObject(this, &UZanshinGameInstance::OnStartOnlineGameComplete);

	/** Bind function for FINDING a Session */
	OnFindSessionsCompleteDelegate = FOnFindSessionsCompleteDelegate::CreateUObject(this, &UZanshinGameInstance::OnFindSessionsComplete);

	/** Bind function for JOINING a Session */
	OnJoinSessionCompleteDelegate = FOnJoinSessionCompleteDelegate::CreateUObject(this, &UZanshinGameInstance::OnJoinSessionComplete);

	/** Bind function for DESTROYING a Session */
	OnDestroySessionCompleteDelegate = FOnDestroySessionCompleteDelegate::CreateUObject(this, &UZanshinGameInstance::OnDestroySessionComplete);

	OnEndSessionCompleteDelegate = FOnEndSessionCompleteDelegate::CreateUObject(this, &UZanshinGameInstance::OnEndSessionComplete);

}

UZanshinGameInstance::~UZanshinGameInstance(){

}

//	Handles button clicks.
void UZanshinGameInstance::HostGameEvent(FString GameName){

	// Creating a local player where we can get the UserID from
	ULocalPlayer* const Player = GetFirstGamePlayer();

	// Call our custom HostSession function. GameSessionName is a GameInstance variable
	HostSession(Player->GetPreferredUniqueNetId(), ZanshinName, true, true, MAX_PLAYERS, GameName);
}

// Find all Hosted Games of this type.
void UZanshinGameInstance::FindGameEvent(){

	ULocalPlayer* const Player = GetFirstGamePlayer();

	FindSessions(Player->GetPreferredUniqueNetId(), ZanshinName, true, true);

}

void UZanshinGameInstance::JoinGameEvent(FString SessionName){
	ULocalPlayer* const Player = GetFirstGamePlayer();

	// Just a SearchResult where we can save the one we want to use, for the case we find more than one!
	FOnlineSessionSearchResult SearchResult;

	// If the Array is not empty, we go through it
	if (SessionSearch->SearchResults.Num() > 0)
	{
		for (int32 i = 0; i < SessionSearch->SearchResults.Num(); i++)
		{
			// To avoid something crazy, we filter sessions from ourself
			if (SessionSearch->SearchResults[i].Session.OwningUserId != Player->GetPreferredUniqueNetId())
			{
				SearchResult = SessionSearch->SearchResults[i];

				// Once we found a Session that is not ours, just join it. Instead of using a for loop, you could
				// use a widget where you click on and have a reference for the GameSession it represents which you can use
				// here
				if (SearchResult.Session.SessionSettings.Settings.FindRef(FName("GAME_NAME")).Data.ToString() == SessionName && SearchResult.Session.SessionSettings.bAllowInvites == true){
					ZJoinSession(Player->GetPreferredUniqueNetId(), ZanshinName, GetResultWithName(SessionName));
					break;
				}
			}
		}
	}
}

// Searches through the array of searchresults to find one with the chosen name.
FOnlineSessionSearchResult UZanshinGameInstance::GetResultWithName(FString SessionName){

	for (int32 i = 0; i < SearchResults.Num(); i++){
		if (SearchResults[i].Session.SessionSettings.Settings.FindRef(FName("GAME_NAME")).Data.ToString() == SessionName)
			return SearchResults[i];
	}

	// If there's no session with that name, host the first one that appears.
	return SearchResults[0];
}

void UZanshinGameInstance::EndSessionEvent(){
	IOnlineSubsystem* OnlineSub = IOnlineSubsystem::Get();
	if (OnlineSub)
	{
		IOnlineSessionPtr Sessions = OnlineSub->GetSessionInterface();

		if (Sessions.IsValid())
		{
			FNamedOnlineSession* Session = Sessions->GetNamedSession(ZanshinName);
			OnEndSessionCompleteDelegateHandle = Sessions->AddOnEndSessionCompleteDelegate_Handle(OnEndSessionCompleteDelegate);

			Sessions->EndSession(ZanshinName);
			if (Session->bHosting){
				DestroySessionEvent();
			}
		}
	}
}

void UZanshinGameInstance::HideSessionEvent(){
	IOnlineSubsystem* OnlineSub = IOnlineSubsystem::Get();
	if (OnlineSub)
	{
		IOnlineSessionPtr Sessions = OnlineSub->GetSessionInterface();

		if (Sessions.IsValid())
		{
			FNamedOnlineSession* Session = Sessions->GetNamedSession(ZanshinName);

			if (Session != nullptr)
				Session->SessionSettings.bAllowInvites = false;
		}
	}
}

void UZanshinGameInstance::ShowSessionEvent(){
	IOnlineSubsystem* OnlineSub = IOnlineSubsystem::Get();
	if (OnlineSub)
	{
		IOnlineSessionPtr Sessions = OnlineSub->GetSessionInterface();

		if (Sessions.IsValid())
		{
			FNamedOnlineSession* Session = Sessions->GetNamedSession(ZanshinName);

			if (Session != nullptr)
				Session->SessionSettings.bAllowInvites = true;
		}
	}
}


void UZanshinGameInstance::DestroySessionEvent()
{
	IOnlineSubsystem* OnlineSub = IOnlineSubsystem::Get();
	if (OnlineSub)
	{
		IOnlineSessionPtr Sessions = OnlineSub->GetSessionInterface();

		if (Sessions.IsValid())
		{
			Sessions->AddOnDestroySessionCompleteDelegate_Handle(OnDestroySessionCompleteDelegate);

			Sessions->DestroySession(ZanshinName);
		}

		OnlineSub->Shutdown();
		OnlineSub->Init();
	}
}

void UZanshinGameInstance::OnEndSessionComplete(FName SessionName, bool bWasSuccessful)
{
	UE_LOG(LogOnline, Log, TEXT("UShooterGameInstance::OnEndSessionComplete: Session=%s bWasSuccessful=%s"), *SessionName.ToString(), bWasSuccessful ? TEXT("true") : TEXT("false"));

	IOnlineSubsystem* OnlineSub = IOnlineSubsystem::Get();
	if (OnlineSub)
	{
		IOnlineSessionPtr Sessions = OnlineSub->GetSessionInterface();
		if (Sessions.IsValid())
		{
			
		}
	}

	// continue
	CleanupOnlineSessionsOnReturnToMainMenu();
}

void UZanshinGameInstance::CleanupOnlineSessionsOnReturnToMainMenu(){
	bool bPendingOnlineOp = false;

	// end online game and then destroy it
	IOnlineSubsystem * OnlineSub = IOnlineSubsystem::Get();
	IOnlineSessionPtr Sessions = (OnlineSub != NULL) ? OnlineSub->GetSessionInterface() : NULL;

	if (Sessions.IsValid())
	{
		EOnlineSessionState::Type SessionState = Sessions->GetSessionState(GameSessionName);
		UE_LOG(LogOnline, Log, TEXT("Session %s is '%s'"), *GameSessionName.ToString(), EOnlineSessionState::ToString(SessionState));

		if (EOnlineSessionState::InProgress == SessionState)
		{
			UE_LOG(LogOnline, Log, TEXT("Ending session %s on return to main menu"), *GameSessionName.ToString());
			OnEndSessionCompleteDelegateHandle = Sessions->AddOnEndSessionCompleteDelegate_Handle(OnEndSessionCompleteDelegate);
			Sessions->EndSession(GameSessionName);
			bPendingOnlineOp = true;
		}
		else if (EOnlineSessionState::Ending == SessionState)
		{
			UE_LOG(LogOnline, Log, TEXT("Waiting for session %s to end on return to main menu"), *GameSessionName.ToString());
			OnEndSessionCompleteDelegateHandle = Sessions->AddOnEndSessionCompleteDelegate_Handle(OnEndSessionCompleteDelegate);
			bPendingOnlineOp = true;
		}
		else if (EOnlineSessionState::Ended == SessionState || EOnlineSessionState::Pending == SessionState)
		{
			UE_LOG(LogOnline, Log, TEXT("Destroying session %s on return to main menu"), *GameSessionName.ToString());
			OnDestroySessionCompleteDelegateHandle = Sessions->AddOnDestroySessionCompleteDelegate_Handle(OnEndSessionCompleteDelegate);
			Sessions->DestroySession(GameSessionName);
			bPendingOnlineOp = true;
		}
		else if (EOnlineSessionState::Starting == SessionState)
		{
			UE_LOG(LogOnline, Log, TEXT("Waiting for session %s to start, and then we will end it to return to main menu"), *GameSessionName.ToString());
			OnStartSessionCompleteDelegateHandle = Sessions->AddOnStartSessionCompleteDelegate_Handle(OnEndSessionCompleteDelegate);
			bPendingOnlineOp = true;
		}
	}

	if (!bPendingOnlineOp)
	{
		//GEngine->HandleDisconnect( GetWorld(), GetWorld()->GetNetDriver() );
	}
}

/*
*
*	UI - HUD
*
*
*/

// A Function for handling the display of UI Widgets.
void UZanshinGameInstance::ShowUserWidget(TSubclassOf<class UUserWidget> wDesiredWidget, bool bIsInputModeUIOnly){
	UWorld* World = GetWorld();
	if (wDesiredWidget && World->GetNetMode() != NM_DedicatedServer && World != nullptr) // Check if the Asset is assigned in the blueprint.
	{
		// Create the widget and store it.
		CurrentWidget = CreateWidget<UUserWidget>(this, wDesiredWidget);

		if (CurrentWidget)
		{
			//MyMainMenu->RemoveFromViewport();
		}

		// now you can use the widget directly since you have a referance for it.
		// Extra check to  make sure the pointer holds the widget.
		if (CurrentWidget)
		{
			//let add it to the view port
			CurrentWidget->AddToViewport();
		}

		
		MyController = World->GetFirstPlayerController();
		
		if (MyController != nullptr){
			if (bIsInputModeUIOnly)
			{
				FInputModeUIOnly Mode;
				Mode.SetWidgetToFocus(CurrentWidget->GetCachedWidget());

				MyController->SetInputMode(Mode);
			}
			else
			{
				FInputModeGameOnly Mode;
				MyController->SetInputMode(Mode);
			}
		}
	}
}

//Adds a Server Row for each search result.
void UZanshinGameInstance::AddServerRows(TArray<FOnlineSessionSearchResult> results)
{
	for (int32 SearchIdx = 0; SearchIdx < results.Num(); SearchIdx++)
	{
		FString ServerName = results[SearchIdx].Session.SessionSettings.Settings.FindRef(FName("GAME_NAME")).Data.ToString();
		FString VersionName = results[SearchIdx].Session.SessionSettings.Settings.FindRef(FName("BUILD_VERSION")).Data.ToString();
		
		if (VersionName == BUILD_VERSION && results[SearchIdx].Session.SessionSettings.bAllowInvites == true){
			this->AddRowToServerList(ServerName, results[SearchIdx].Session.NumOpenPublicConnections);
		}
	}
}



/*
*
*	NETWORK
*
*
*/

/* ------- HOST GAME ------- */

//Function to host a session.
bool UZanshinGameInstance::HostSession(TSharedPtr<const FUniqueNetId> UserId, FName SessionName, bool bIsLAN, bool bIsPresence, int32 MaxNumPlayers, FString UserGameName)
{
	// Get the Online Subsystem to work with
	IOnlineSubsystem* const OnlineSub = IOnlineSubsystem::Get();

	if (OnlineSub)
	{
		// Get the Session Interface, so we can call the "CreateSession" function on it
		IOnlineSessionPtr Sessions = OnlineSub->GetSessionInterface();

		if (Sessions.IsValid() && UserId.IsValid())
		{
			/*
			Fill in all the Session Settings that we want to use.

			There are more with SessionSettings.Set(...);
			For example the Map or the GameMode/Type.
			*/
			SessionSettings = MakeShareable(new FOnlineSessionSettings());
			
			SessionSettings->bIsLANMatch = bIsLAN;
			SessionSettings->bUsesPresence = bIsPresence;
			SessionSettings->NumPublicConnections = MaxNumPlayers;
			SessionSettings->NumPrivateConnections = 0;
			SessionSettings->bAllowInvites = true;
			SessionSettings->bAllowJoinInProgress = true;
			SessionSettings->bShouldAdvertise = true;
			SessionSettings->bAllowJoinViaPresence = true;
			SessionSettings->bAllowJoinViaPresenceFriendsOnly = false;
			
			FOnlineSessionSetting CurrentGameName;
			CurrentGameName.AdvertisementType = EOnlineDataAdvertisementType::ViaOnlineService;
			CurrentGameName.Data = UserGameName;

			FOnlineSessionSetting CurrentGameVersion;
			CurrentGameVersion.AdvertisementType = EOnlineDataAdvertisementType::ViaOnlineService;
			CurrentGameVersion.Data = BuildVersion.ToString();
			
			SessionSettings->Settings.Add(FName("GAME_NAME"), CurrentGameName);
			SessionSettings->Settings.Add(FName("BUILD_VERSION"), CurrentGameVersion);

		
			SessionSettings->Set(SETTING_MAPNAME, FString("Session_Lobby"), EOnlineDataAdvertisementType::ViaOnlineService);
			


			// Set the delegate to the Handle of the SessionInterface
			OnCreateSessionCompleteDelegateHandle = Sessions->AddOnCreateSessionCompleteDelegate_Handle(OnCreateSessionCompleteDelegate);

			// Our delegate should get called when this is complete (doesn't need to be successful!)
			return Sessions->CreateSession(*UserId, SessionName, *SessionSettings);
		}
	}
	else
	{
		GEngine->AddOnScreenDebugMessage(0, 10.f, FColor::Red, "FAILED!!!");
	}

	return false;
}

//Delegate called when create session is complete. Sesion is just an object, not a game.
void UZanshinGameInstance::OnCreateSessionComplete(FName SessionName, bool bWasSuccessful)
{
	GEngine->AddOnScreenDebugMessage(0, 5.f, FColor::Red, "SessionCreateComplete");

	// Get the OnlineSubsystem so we can get the Session Interface
	IOnlineSubsystem* OnlineSub = IOnlineSubsystem::Get();
	if (OnlineSub)
	{
		// Get the Session Interface to call the StartSession function
		IOnlineSessionPtr Sessions = OnlineSub->GetSessionInterface();

		if (Sessions.IsValid())
		{
			// Clear the SessionComplete delegate handle, since we finished this call
			Sessions->ClearOnCreateSessionCompleteDelegate_Handle(OnCreateSessionCompleteDelegateHandle);
			if (bWasSuccessful)
			{
				// Set the StartSession delegate handle
				OnStartSessionCompleteDelegateHandle = Sessions->AddOnStartSessionCompleteDelegate_Handle(OnStartSessionCompleteDelegate);

				// Our StartSessionComplete delegate should get called after this
				Sessions->StartSession(SessionName);
			}
		}

	}
}

//Delegate called when creating the game creation was succesful.
void UZanshinGameInstance::OnStartOnlineGameComplete(FName SessionName, bool bWasSuccessful)
{
	// Get the Online Subsystem so we can get the Session Interface
	IOnlineSubsystem* OnlineSub = IOnlineSubsystem::Get();
	if (OnlineSub)
	{
		// Get the Session Interface to clear the Delegate
		IOnlineSessionPtr Sessions = OnlineSub->GetSessionInterface();

		if (Sessions.IsValid())
		{
			// Clear the delegate, since we are done with this call
			Sessions->ClearOnStartSessionCompleteDelegate_Handle(OnStartSessionCompleteDelegateHandle);
		}
	}

	// If the start was successful, we can open a NewMap if we want. Make sure to use "listen" as a parameter!
	if (bWasSuccessful)
	{
		UGameplayStatics::OpenLevel(GetWorld(), "Session_Lobby", true, "listen");
		//BeginPlayLobby();
		OnClientJoinGame();
	}

	//Input Mode to Game and UI.
	FInputModeUIOnly Mode2;
	MyController->SetInputMode(Mode2);

}

/* ------- HOST GAME END ------- */

/* ------- FIND GAME START ------- */

void UZanshinGameInstance::FindSessions(TSharedPtr<const FUniqueNetId> UserId, FName SessionName, bool bIsLAN, bool bIsPresence)
{
	// Get the OnlineSubsystem we want to work with
	IOnlineSubsystem* OnlineSub = IOnlineSubsystem::Get();

	if (OnlineSub)
	{
		// Get the SessionInterface from our OnlineSubsystem
		IOnlineSessionPtr Sessions = OnlineSub->GetSessionInterface();



		if (Sessions.IsValid() && UserId.IsValid())
		{
			/*
			Fill in all the SearchSettings, like if we are searching for a LAN game and how many results we want to have!
			*/
			SessionSearch = MakeShareable(new FOnlineSessionSearch());

			SessionSearch->bIsLanQuery = bIsLAN;
			SessionSearch->MaxSearchResults = 20;
			SessionSearch->PingBucketSize = 50;

			// We only want to set this Query Setting if "bIsPresence" is true
			if (bIsPresence)
			{
				SessionSearch->QuerySettings.Set(SEARCH_PRESENCE, bIsPresence, EOnlineComparisonOp::Equals);
			}

			TSharedRef<FOnlineSessionSearch> SearchSettingsRef = SessionSearch.ToSharedRef();

			// Set the Delegate to the Delegate Handle of the FindSession function
			OnFindSessionsCompleteDelegateHandle = Sessions->AddOnFindSessionsCompleteDelegate_Handle(OnFindSessionsCompleteDelegate);

			// Finally call the SessionInterface function. The Delegate gets called once this is finished
			Sessions->FindSessions(*UserId, SearchSettingsRef);
		}
	}
	else
	{
		// If something goes wrong, just call the Delegate Function directly with "false".
		OnFindSessionsComplete(false);
	}
}

void UZanshinGameInstance::OnFindSessionsComplete(bool bWasSuccessful)
{
	//GEngine->AddOnScreenDebugMessage(0, 10.f, FColor::Red, "FIND SESSION COMPLETE");

	// Get OnlineSubsystem we want to work with
	IOnlineSubsystem* const OnlineSub = IOnlineSubsystem::Get();
	if (OnlineSub)
	{
		// Get SessionInterface of the OnlineSubsystem
		IOnlineSessionPtr Sessions = OnlineSub->GetSessionInterface();
		if (Sessions.IsValid())
		{
			// Clear the Delegate handle, since we finished this call
			Sessions->ClearOnFindSessionsCompleteDelegate_Handle(OnFindSessionsCompleteDelegateHandle);

			//If we have found at least 1 session, we just going to debug them. You could add them to a list of UMG Widgets, like it is done in the BP version!
			if (SessionSearch->SearchResults.Num() > 0)
			{

				AddServerRows(SessionSearch->SearchResults);

				// "SessionSearch->SearchResults" is an Array that contains all the information. You can access the Session in this and get a lot of information.
				// This can be customized later on with your own classes to add more information that can be set and displayed
				for (int32 SearchIdx = 0; SearchIdx < SessionSearch->SearchResults.Num(); SearchIdx++)
				{
					SearchResults.Add(SessionSearch->SearchResults[SearchIdx]);
					
					// OwningUserName is just the SessionName for now. I guess you can create your own Host Settings class and GameSession Class and add a proper GameServer Name here.
					// This is something you can't do in Blueprint for example!
				}
			}
		}
	}
}

/* ------- FIND GAME END ------- */

/* ------- JOIN GAME START ------- */

bool UZanshinGameInstance::ZJoinSession(TSharedPtr<const FUniqueNetId> UserId, FName SessionName, const FOnlineSessionSearchResult& SearchResult)
{
	// Return bool
	bool bSuccessful = false;

	// Get OnlineSubsystem we want to work with
	IOnlineSubsystem* OnlineSub = IOnlineSubsystem::Get();

	if (OnlineSub)
	{
		// Get SessionInterface from the OnlineSubsystem
		IOnlineSessionPtr Sessions = OnlineSub->GetSessionInterface();

		if (Sessions.IsValid() && UserId.IsValid())
		{
			// Set the Handle again
			OnJoinSessionCompleteDelegateHandle = Sessions->AddOnJoinSessionCompleteDelegate_Handle(OnJoinSessionCompleteDelegate);

			// Call the "JoinSession" Function with the passed "SearchResult". The "SessionSearch->SearchResults" can be used to get such a
			// "FOnlineSessionSearchResult" and pass it. Pretty straight forward!
			bSuccessful = Sessions->JoinSession(*UserId, SessionName, SearchResult);
		}
	}

	return bSuccessful;
}

void UZanshinGameInstance::OnJoinSessionComplete(FName SessionName, EOnJoinSessionCompleteResult::Type Result)
{
	// Get the OnlineSubsystem we want to work with
	IOnlineSubsystem* OnlineSub = IOnlineSubsystem::Get();
	if (OnlineSub)
	{
		// Get SessionInterface from the OnlineSubsystem
		IOnlineSessionPtr Sessions = OnlineSub->GetSessionInterface();

		if (Sessions.IsValid())
		{
			// Clear the Delegate again
			Sessions->ClearOnJoinSessionCompleteDelegate_Handle(OnJoinSessionCompleteDelegateHandle);

			// Get the first local PlayerController, so we can call "ClientTravel" to get to the Server Map
			// This is something the Blueprint Node "Join Session" does automatically!
			APlayerController * const PlayerController = GetFirstLocalPlayerController();
			PlayerController->SetAutonomousProxy(true);
			// We need a FString to use ClientTravel and we can let the SessionInterface contruct such a
			// String for us by giving him the SessionName and an empty String. We want to do this, because
			// Every OnlineSubsystem uses different TravelURLs
			FString TravelURL;

			if (PlayerController && Sessions->GetResolvedConnectString(SessionName, TravelURL))
			{
	
				GEngine->AddOnScreenDebugMessage(0, 10.f, FColor::Red, TravelURL);
				// Finally call the ClienTravel. If you want, you could print the TravelURL to see
				// how it really looks like
				PlayerController->ClientTravel(TravelURL, ETravelType::TRAVEL_Absolute);

				OnClientJoinGame();
				//BeginPlayLobby();

				//Input Mode to Game and UI.
				FInputModeUIOnly Mode2;
				PlayerController->SetInputMode(Mode2);
			}
		}
	}
}

/* ------- JOIN GAME END ------- */

/* ------- DESTROY SESSION ------- */
void UZanshinGameInstance::OnDestroySessionComplete(FName SessionName, bool bWasSuccessful)
{
	// Get the OnlineSubsystem we want to work with
	IOnlineSubsystem* OnlineSub = IOnlineSubsystem::Get();
	if (OnlineSub)
	{
		UWorld* const World = GetWorld();
		AZanshinGameState* const GameState = World != NULL ? World->GetGameState<AZanshinGameState>() : NULL;

		if (GameState) {
			// Give the game state a chance to cleanup first
			GameState->RequestFinishAndExitToMainMenu();
		}

		// Get the SessionInterface from the OnlineSubsystem
		IOnlineSessionPtr Sessions = OnlineSub->GetSessionInterface();

		if (Sessions.IsValid())
		{
			// Clear the Delegate
			Sessions->ClearOnDestroySessionCompleteDelegate_Handle(OnDestroySessionCompleteDelegateHandle);

			// If it was successful, we just load another level (could be a MainMenu!)
			if (bWasSuccessful)
			{
				UGameplayStatics::OpenLevel(GetWorld(), "MainMenu", true);
			}
		}
	}
}

// When a client joins the lobby.
void UZanshinGameInstance::OnClientJoinLobby()
{
	APlayerController * const ClientPlayerController = GetFirstLocalPlayerController();
	AZanshinPlayerState* ClientPlayerState = Cast<AZanshinPlayerState>(ClientPlayerController->PlayerState);
	if (ClientPlayerController != nullptr && ClientPlayerState != nullptr)
	{
		AZanshinPlayerController* LocalPlayerController = Cast<AZanshinPlayerController>(ClientPlayerController);
		LocalPlayerController->ServerOnJoinedLobby(ClientPlayerState, PlayerName);
	}
}

//In Player and Team.
void UZanshinGameInstance::RegisterPlayer(AZanshinPlayerState* ZPlayerState, int32 Team){
	if (PlayerTeams.Contains(ZPlayerState->UniqueId))
		PlayerTeams[ZPlayerState->UniqueId] = Team;
	else
		PlayerTeams.Add(ZPlayerState->UniqueId, Team);
}

//In Player and Name.
void UZanshinGameInstance::RegisterPlayerName(AZanshinPlayerState* ZPlayerState, FName Name){
	if (PlayerNames.Contains(ZPlayerState->UniqueId))
		PlayerNames[ZPlayerState->UniqueId] = Name;
	else
		PlayerNames.Add(ZPlayerState->UniqueId, Name);
}

FPlayerStatsData UZanshinGameInstance::GetData(){
	return LocalPlayerData;
}

void UZanshinGameInstance::SetPlayerData(FPlayerStatsData Stats){
	LocalPlayerData = FPlayerStatsData();
	LocalPlayerData = Stats;
}

TArray<FZanshinEndOfMatchInfo> UZanshinGameInstance::GetRedTeam(){
	PlayerTeams.Empty();
	PlayerNames.Empty();
	PlayerName = FName(TEXT("DefaultUsername"));
	PlayerTeamNumber = 0;
	return RedTeam;
}

void UZanshinGameInstance::SetRedTeam(FZanshinTeam InRedTeam){
	RedTeam.Empty();
	for (APlayerState* NewState : InRedTeam.TeamPlayers){
		if (NewState != nullptr){
			AZanshinPlayerState* PState = Cast<AZanshinPlayerState>(NewState);
			if (PState != nullptr){
				FZanshinEndOfMatchInfo NewInfo;
				NewInfo.PlayerDeaths = PState->NumDeaths;
				NewInfo.PlayerKills = PState->KillCount;
				NewInfo.PlayerName = PState->UserName.ToString();
				NewInfo.PlayerAssists = PState->KillAssist;
				NewInfo.PlayerScore = PState->Score;
				RedTeam.Add(NewInfo);
			}
		}
	}
	RedTeamControlPointScore = InRedTeam.NonKillRelatedScore;
}

TArray<FZanshinEndOfMatchInfo> UZanshinGameInstance::GetBlueTeam(){
	PlayerTeams.Empty();
	PlayerNames.Empty();
	PlayerName = FName(TEXT("DefaultUsername"));
	PlayerTeamNumber = 0;
	return BlueTeam;
}

void UZanshinGameInstance::SetBlueTeam(FZanshinTeam InBlueTeam){
	BlueTeam.Empty();
	for (APlayerState* NewState : InBlueTeam.TeamPlayers){
		if (NewState != nullptr){
			AZanshinPlayerState* PState = Cast<AZanshinPlayerState>(NewState);
			if (PState != nullptr){
				FZanshinEndOfMatchInfo NewInfo;
				NewInfo.PlayerDeaths = PState->NumDeaths;
				NewInfo.PlayerKills = PState->KillCount;
				NewInfo.PlayerName = PState->UserName.ToString();
				NewInfo.PlayerAssists = PState->KillAssist;
				NewInfo.PlayerScore = PState->Score;
				BlueTeam.Add(NewInfo);
			}
		}
	}
	BlueTeamControlPointScore = InBlueTeam.NonKillRelatedScore;
}