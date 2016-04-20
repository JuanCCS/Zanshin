// Fill out your copyright notice in the Description page of Project Settings.

#include "Zanshin.h"
#include "ZanshinPlayerState.h"
#include "ZanshinPlayerController.h"
#include "GameMode/ZanshinGameInstance.h"
#include "Networking/NetworkSingleton.h"

void AZanshinPlayerState::PlayerKill(FString KilledPlayerName, int32 KillValue)
{
	KillCount++;
	Score += KillValue;
}

void AZanshinPlayerState::OnRep_KillCount() { }

void AZanshinPlayerState::OnRep_IsReadyLobby() { }

void AZanshinPlayerState::GetLifetimeReplicatedProps(TArray< FLifetimeProperty > & OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(AZanshinPlayerState, KillCount);
	DOREPLIFETIME(AZanshinPlayerState, KillAssist);
	DOREPLIFETIME(AZanshinPlayerState, UserName);
	DOREPLIFETIME(AZanshinPlayerState, TeamNumber);
	DOREPLIFETIME(AZanshinPlayerState, bPlayerIsReady);
	DOREPLIFETIME(AZanshinPlayerState, bPlayerScoredKill);
	DOREPLIFETIME(AZanshinPlayerState, bIsReadyLobby);
	DOREPLIFETIME(AZanshinPlayerState, NumDeaths);
	DOREPLIFETIME(AZanshinPlayerState, ZanshinPlayerData);
	DOREPLIFETIME(AZanshinPlayerState, CurrentKillStreak);
}

//Changing Team Number.
void AZanshinPlayerState::ServerSetTeamNumber_Implementation(int32 NewTeamNumber)
{
	TeamNumber = NewTeamNumber;
	UZanshinGameInstance* ZanshinGameInstance = (UZanshinGameInstance*)GetWorld()->GetGameInstance();
	ZanshinGameInstance->RegisterPlayer(this, TeamNumber);
	OnRep_TeamNumber();
}
bool AZanshinPlayerState::ServerSetTeamNumber_Validate(int32 NewTeamNumber) { return true; }

////////////////////////////////////////////////////////////////////////////////
//// Score, Kill, Death

void AZanshinPlayerState::ScoreKill(AZanshinPlayerState* Victim, int32 Points)
{
	KillCount++;
	CurrentKillStreak++;

	if (KillStreakActive > 0 && CurrentKillStreak % KillStreakActive == 0 && CurrentKillStreak < KillStreakLimit) {
		FMath::Clamp<int32>(CurrentKillStreak, 0, KillStreakLimit);
		Client_KillstreakActive(CurrentKillStreak);
	}

	bPlayerScoredKill = true;
	ScorePoints(Points);
}

void AZanshinPlayerState::Client_KillstreakActive_Implementation(int32 KillstreakCount) { KillstreakActive(KillstreakCount); }
bool AZanshinPlayerState::Client_KillstreakActive_Validate(int32 KillstreakCount) { return true; }

void AZanshinPlayerState::Client_ProvideAsisstFeedback_Implementation() { 
	/*UWorld* World = GetWorld();
	if (World != nullptr){
		AZanshinCharacter* Char = Cast<AZanshinCharacter>(World->GetFirstPlayerController()->GetPawn());
		if (Char != nullptr && Char->CharacterAudioManager->KillingSpree.Num() > Char->characterIndex){
			Char->Client_PlaySound_Attached(Char->CharacterAudioManager->KillingSpree[Char->characterIndex]);
		}
	}*/
	BP_Event_ShowAssistFeedback();
}
bool AZanshinPlayerState::Client_ProvideAsisstFeedback_Validate() { return true; }


void AZanshinPlayerState::ScoreAssist(int32 Points)
{
	KillAssist++;
	Client_ProvideAsisstFeedback();
	ScorePoints(Points);
}

void AZanshinPlayerState::ScoreDeath(AZanshinPlayerState* KilledBy, int32 Points)
{
	NumDeaths++;
	ScorePoints(Points);
	CurrentKillStreak = 0;
}

void AZanshinPlayerState::ScorePoints(int32 Points)
{
	if (Role < ROLE_Authority){
		ServerScorePoints(Points);
	}
	else
	{
		AZanshinGameState* const MyGameState = Cast<AZanshinGameState>(GetWorld()->GameState);
		if (MyGameState && TeamNumber >= 0)
		{
			//MyGameState->NetMulticast_UpdateScore();
		}
		Score += Points;
	}
}

void AZanshinPlayerState::KillstreakActive(int32 KillstreakCount) 
{
	BP_Event_KillstreakActive(KillstreakCount);
	Client_PlayKillStreakSound(KillstreakCount);
}

void AZanshinPlayerState::InformAboutKill_Implementation(class AZanshinPlayerState* KillerPlayerState, const UDamageType* KillerDamageType, class AZanshinPlayerState* KilledPlayerState)
{
	//id can be null for bots
	if (KillerPlayerState->UniqueId.IsValid())
	{
		//search for the actual killer before calling OnKill()	
		for (FConstPlayerControllerIterator It = GetWorld()->GetPlayerControllerIterator(); It; ++It)
		{
			AZanshinPlayerController* TestPC = Cast<AZanshinPlayerController>(*It);
			if (TestPC && TestPC->IsLocalController())
			{
				// a local player might not have an ID if it was created with CreateDebugPlayer.
				ULocalPlayer* LocalPlayer = Cast<ULocalPlayer>(TestPC->Player);
				TSharedPtr<const FUniqueNetId> LocalID = LocalPlayer->GetCachedUniqueNetId();
				if (LocalID.IsValid() && *LocalPlayer->GetCachedUniqueNetId() == *KillerPlayerState->UniqueId)
				{
					//TestPC->OnKill();
				}
			}
		}
	}
}

void AZanshinPlayerState::BroadcastDeath_Implementation(class AZanshinPlayerState* KillerPlayerState, const UDamageType* KillerDamageType, class AZanshinPlayerState* KilledPlayerState)
{
	for (FConstPlayerControllerIterator It = GetWorld()->GetPlayerControllerIterator(); It; ++It)
	{
		// all local players get death messages so they can update their huds.
		AZanshinPlayerController* TestPC = Cast<AZanshinPlayerController>(*It);
		if (TestPC && TestPC->IsLocalController())
		{
			//TestPC->OnDeathMessage(KillerPlayerState, this, KillerDamageType);
		}
	}
}

//Assign Username on the Server.
void AZanshinPlayerState::ServerSetUserName_Implementation(FName NewUserName)
{
	UserName = NewUserName;
}

bool AZanshinPlayerState::ServerSetUserName_Validate(FName NewUserName) { return true; }

//Set Ready on Lobby.
void AZanshinPlayerState::ServerSetReady_Implementation(bool ReadyState)
{
	bIsReadyLobby = ReadyState;
	AZanshinPlayerController* MyPlayerController = Cast<AZanshinPlayerController>(GetGameInstance()->GetFirstLocalPlayerController());
	MyPlayerController->CheckReadyState();
}

bool AZanshinPlayerState::ServerSetReady_Validate(bool ReadyState) { return true; }

// Getting the PlayerWidget On Screen.
bool AZanshinPlayerState::Client_AddPlayerWidget_Validate(FName NetUserName, bool NetIsReady, int32 NetTeamNumber) { return true; }

void AZanshinPlayerState::Client_AddPlayerWidget_Implementation(FName NetUserName, bool NetIsReady, int32 NetTeamNumber)
{
	AddPlayerWidget(NetUserName, NetIsReady, NetTeamNumber);
}

// Add Player Widget.
void AZanshinPlayerState::AddPlayerWidget(FName NetUserName, bool NetIsReady, int32 NetTeamNumber)
{
	UZanshinGameInstance* ZanshinGameInstance = Cast<UZanshinGameInstance>(GetWorld()->GetGameInstance());
	ZanshinGameInstance->AddPlayerWidget(NetUserName, NetIsReady, NetTeamNumber);
}

void AZanshinPlayerState::ServerScorePoints_Implementation(int32 Points) { ScorePoints(Points); }
bool AZanshinPlayerState::ServerScorePoints_Validate(int32 Points) { return true; }

void AZanshinPlayerState::Server_KillstreakActive_Implementation(int32 KillstreakCount) { KillstreakActive(KillstreakCount); }
bool AZanshinPlayerState::Server_KillstreakActive_Validate(int32 KillstreakCount) { return true; }

void AZanshinPlayerState::SetTeamNumberByBP(int32 InTeamNumber){
	UZanshinGameInstance* ZanshinGameInstance = Cast<UZanshinGameInstance>(GetWorld()->GetGameInstance());
	ZanshinGameInstance->PlayerTeamNumber = InTeamNumber;
	ServerSetTeamNumber(InTeamNumber);
}

int32 AZanshinPlayerState::GetTeamNumber()
{
	Server_GetTeamNumber();
	return TeamNumber;
}

void AZanshinPlayerState::Server_GetTeamNumber_Implementation()
{
	UZanshinGameInstance* ZanshinGameInstance = Cast<UZanshinGameInstance>(GetWorld()->GetGameInstance());
	TeamNumber = ZanshinGameInstance->PlayerTeams[UniqueId];
}
bool AZanshinPlayerState::Server_GetTeamNumber_Validate() { return true; }

void AZanshinPlayerState::ResetScoreKill() 
{
	if (Role < ROLE_Authority) {
		Server_ResetScoreKill();
	} else {
		bPlayerScoredKill = false;
	}
}

void AZanshinPlayerState::Server_ResetScoreKill_Implementation() { ResetScoreKill(); }
bool AZanshinPlayerState::Server_ResetScoreKill_Validate() { return true; }

void AZanshinPlayerState::SavePlayerData()
{
	UZanshinGameInstance* ZanshinGameInstance = Cast<UZanshinGameInstance>(GetWorld()->GetGameInstance());
	ZanshinGameInstance->SetPlayerData(ZanshinPlayerData);
}

// Getting the PlayerWidget On Screen.
bool AZanshinPlayerState::Client_SavePlayerData_Validate() { return true; }
void AZanshinPlayerState::Client_SavePlayerData_Implementation() { SavePlayerData(); }

bool AZanshinPlayerState::Client_SaveTeamData_Validate(FZanshinTeam RedTeam, FZanshinTeam BlueTeam) { return true; }
void AZanshinPlayerState::Client_SaveTeamData_Implementation(FZanshinTeam RedTeam, FZanshinTeam BlueTeam) { SaveTeamData(RedTeam, BlueTeam); }

void AZanshinPlayerState::SaveTeamData(FZanshinTeam RedTeam, FZanshinTeam BlueTeam){
	UZanshinGameInstance* ZanshinGameInstance = Cast<UZanshinGameInstance>(GetWorld()->GetGameInstance());
	if (ZanshinGameInstance != nullptr){
		ZanshinGameInstance->SetRedTeam(RedTeam);
		ZanshinGameInstance->SetBlueTeam(BlueTeam);
	}
}

void AZanshinPlayerState::NotifyJoinedMap() { bPlayerIsInMap = true; }

bool AZanshinPlayerState::Client_PlayKillStreakSound_Validate(int32 KillStreakNumber) { return true; }
void AZanshinPlayerState::Client_PlayKillStreakSound_Implementation(int32 KillStreakNumber) { PlayKillStreakSound(KillStreakNumber); }

void AZanshinPlayerState::PlayKillStreakSound(int32 KillStreakNumber){
	UWorld* World = GetWorld();
	if (World != nullptr){
		AZanshinPlayerController* Client_ZanshinPlayerController = Cast<AZanshinPlayerController>(World->GetGameInstance()->GetFirstLocalPlayerController());
		if (Client_ZanshinPlayerController != nullptr){
			AZanshinCharacter* Client_Character = Cast<AZanshinCharacter>(Client_ZanshinPlayerController->GetPawn());
			if (Client_Character != nullptr){
				Client_Character->Client_PlaySound_Attached(Client_Character->CharacterAudioManager->KillingSpree[Client_Character->characterIndex]);
			}
		}
	}
}