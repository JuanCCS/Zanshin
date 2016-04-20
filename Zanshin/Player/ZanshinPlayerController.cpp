// Fill out your copyright notice in the Description page of Project Settings.

#include "Zanshin.h"
#include "GameMode/ZanshinGameInstance.h"
#include "GameMode/ZanshinGameState.h"
#include "ZanshinCharacter.h"
#include "ZanshinPlayerState.h"
#include "ZanshinPlayerController.h"
#include "Environment/ZanshinCameraSpectator.h"

AZanshinPlayerController::AZanshinPlayerController()
{
	bIsDisplayingScore = false;
}

void AZanshinPlayerController::SetupInputComponent()
{
	Super::SetupInputComponent();

	//////////////////////////////////////////////////////////////////////////
	// Display Score

	InputComponent->BindAction("DisplayScore", IE_Pressed, this, &AZanshinPlayerController::DisplayScore);
	InputComponent->BindAction("DisplayScore", IE_Released, this, &AZanshinPlayerController::HideScore);

	InputComponent->BindAction("Spectator", IE_Pressed, this, &AZanshinPlayerController::Spectator);
}

void AZanshinPlayerController::GetLifetimeReplicatedProps(TArray< FLifetimeProperty > & OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(AZanshinPlayerController, CharacterTeam);
	DOREPLIFETIME(AZanshinPlayerController, PlayerName);
	DOREPLIFETIME(AZanshinPlayerController, bGoKillerCamera);
	DOREPLIFETIME(AZanshinPlayerController, Enemy);	
	DOREPLIFETIME(AZanshinPlayerController, bIsSpectator);
	DOREPLIFETIME(AZanshinPlayerController, SpectatorClass);
}

void AZanshinPlayerController::PostInitializeComponents()
{
	Super::PostInitializeComponents();
}

void AZanshinPlayerController::BeginPlay()
{
	Super::BeginPlay();

	for (FActorIterator Itr(GetWorld()); Itr; ++Itr)
	{
		AZanshinCameraSpectator* CameraSpot = Cast<AZanshinCameraSpectator>(*Itr);
		if (CameraSpot != nullptr) {			
			CameraSpots.AddUnique(CameraSpot);
		}
	}
	
	Server_SetJoinedMap();

	bIsEndGame = false;
	bIsSpectator = false;
}

void AZanshinPlayerController::SetupInfo()
{
	UZanshinGameInstance* ZGameInstance = Cast<UZanshinGameInstance>(GetGameInstance());
	if (ZGameInstance != NULL)
	{
		PlayerName = ZGameInstance->PlayerName;
		CharacterTeam = ZGameInstance->PlayerTeamNumber;
		Server_SetupInfo(PlayerName, CharacterTeam);
		ServerRestartPlayer();
	}
}

void AZanshinPlayerController::TickActor(float DeltaTime, enum ELevelTick TickType, FActorTickFunction& ThisTickFunction) { Super::TickActor(DeltaTime, TickType, ThisTickFunction); }

void AZanshinPlayerController::UnFreeze() 
{ 
	if (!bIsEndGame) {
		ServerRestartPlayer();
	}

	BP_Event_RemoveHUDDeathCamera();
}

void AZanshinPlayerController::PawnPendingDestroy(APawn* P)
{
	LastDeathLocation = P->GetActorLocation();
	FVector CameraLocation = LastDeathLocation + FVector(0, 0, 300.0f);
	FRotator CameraRotation(-90.0f, 0.0f, 0.0f);
	FindDeathCameraSpot(CameraLocation, CameraRotation);

	Super::PawnPendingDestroy(P);

	ClientSetSpectatorCamera(CameraLocation, CameraRotation);

	if (Role < ROLE_Authority){
		if (bGoKillerCamera) {
			GetWorld()->GetTimerManager().SetTimer(TimerHandle_ChangeToSpectatorMapCamera, this, &AZanshinPlayerController::Server_GoKillerCamera, TimeToSwitchToSpectatorCamera, false);
		} else {
			GetWorld()->GetTimerManager().SetTimer(TimerHandle_ChangeToSpectatorMapCamera, this, &AZanshinPlayerController::Server_SetSpectatorCameraForMap, TimeToSwitchToSpectatorCamera, false);
		}
	} else {
		if (bGoKillerCamera) {
			GetWorld()->GetTimerManager().SetTimer(TimerHandle_ChangeToSpectatorMapCamera, this, &AZanshinPlayerController::ClientGoToKillerCamera, TimeToSwitchToSpectatorCamera, false);
		} else {
			GetWorld()->GetTimerManager().SetTimer(TimerHandle_ChangeToSpectatorMapCamera, this, &AZanshinPlayerController::ClientSetSpectatorCameraForMap, TimeToSwitchToSpectatorCamera, false);
		}
	}
}

void AZanshinPlayerController::ClientSetSpectatorCamera_Implementation(FVector CameraLocation, FRotator CameraRotation)
{
	UpdateMeshVisibility(false);
	SetInitialLocationAndRotation(CameraLocation, CameraRotation);
	SetViewTarget(this);
}

void AZanshinPlayerController::ClientSetSpectatorCameraForMap()
{
	int32 CameraIndex = FMath::RandHelper(CameraSpots.Num());
	if (CameraIndex < CameraSpots.Num() && CameraSpots[CameraIndex] != nullptr)
	{
		SetInitialLocationAndRotation(CameraSpots[CameraIndex]->GetActorLocation(),
			CameraSpots[CameraIndex]->GetActorRotation());

		SetViewTarget(this);
	}
}

void AZanshinPlayerController::ClientGoToKillerCamera_Implementation()
{
	if (Enemy != nullptr)
	{
		//BP_Event_GoKillerCamera(Enemy);
		/*Enemy->ChangeCameraActive(false);
		ClientSetViewTarget(Enemy);
		Enemy->ChangeCameraActive(true);*/

		/*SetInitialLocationAndRotation(Enemy->GetActorLocation() + Enemy->GetActorForwardVector()*150.f, Enemy->GetActorRotation().GetInverse());
		UpdateMeshVisibility(true);*/
		BP_Event_GoKillerCamera(Enemy);
		/*SetViewTarget(Enemy);*/


	} else {
		if (Role < ROLE_Authority) {
			Server_SetSpectatorCameraForMap();
		} else {
			ClientSetSpectatorCameraForMap();
		}
	}
}

bool AZanshinPlayerController::FindDeathCameraSpot(FVector& CameraLocation, FRotator& CameraRotation)
{
	const FVector PawnLocation = GetPawn()->GetActorLocation();
	PawnLocationSpectator = PawnLocation;
	FRotator ViewDir = GetControlRotation();
	ViewDir.Pitch = -45.0f;

	const float YawOffsets[] = { 0.0f, -180.0f, 90.0f, -90.0f, 45.0f, -45.0f, 135.0f, -135.0f };
	const float CameraOffset = 600.0f;
	FCollisionQueryParams TraceParams(TEXT("DeathCamera"), true, GetPawn());

	FHitResult HitResult;
	for (int32 i = 0; i < ARRAY_COUNT(YawOffsets); i++)
	{
		FRotator CameraDir = ViewDir;
		CameraDir.Yaw += YawOffsets[i];
		CameraDir.Normalize();

		const FVector TestLocation = PawnLocation - CameraDir.Vector() * CameraOffset;

		const bool bBlocked = GetWorld()->LineTraceSingleByChannel(HitResult, PawnLocation, TestLocation, ECC_Camera, TraceParams);

		if (!bBlocked)
		{
			CameraLocation = TestLocation;
			CameraRotation = CameraDir;
			return true;
		}
	}

	return false;
}

void AZanshinPlayerController::EndGameCamera()
{
	bIsEndGame = true;

	UnPossess();

	ClientSetSpectatorCameraForMap();

}

void AZanshinPlayerController::DisplayScore() { bIsDisplayingScore = true; }
void AZanshinPlayerController::HideScore() { bIsDisplayingScore = false; }

void AZanshinPlayerController::KilledBy(class AZanshinCharacter* KillerEnemy)
{
	if (KillerEnemy != nullptr) {
		Enemy = KillerEnemy;
	} else {
		Enemy = nullptr;
	}
}

void AZanshinPlayerController::Spectator()
{
	if (Role < ROLE_Authority) {
		Server_SetSpectator();
	} else {
		bIsSpectator = true;
		//GetControlledPawn()->Destroy();
		ActivateSpectatorPawn();
	}
}

void AZanshinPlayerController::ActivateSpectatorPawn()
{
	if (Role < ROLE_Authority) {
		Server_SetSpectator();
	} else {
		UnPossess();

		UWorld* const World = GetWorld();
		if (World != nullptr) {
			AZanshinSpectatorPawn* Spectator = World->SpawnActor<AZanshinSpectatorPawn>(SpectatorClass, FVector::ZeroVector, FRotator::ZeroRotator);
			Spectator->SetActorLocation(PawnLocationSpectator);
			Possess(Cast<APawn>(Spectator));
		}

		BP_Event_RemoveHUDDeathCamera();

		bIsSpectator = true;
	}
}

///////////////////////////////////////////////////////////////////////
// NETWORK

void AZanshinPlayerController::OnJoinedLobby(APlayerState* InPlayerState, FName InPlayerName)
{
	AZanshinPlayerState* NewPlayerState = Cast<AZanshinPlayerState>(InPlayerState);
	check(NewPlayerState);

	if (NewPlayerState != nullptr)
	{
		NewPlayerState->UserName = InPlayerName;
		UZanshinGameInstance* ZGameInstance = Cast<UZanshinGameInstance>(GetGameInstance());
		ZGameInstance->RegisterPlayerName(NewPlayerState, InPlayerName);
		CreatePlayerWidgets(NewPlayerState);
	}
}

void AZanshinPlayerController::CreatePlayerWidgets(AZanshinPlayerState* InPlayerState)
{
	UWorld* World = GetWorld();
	if (World != nullptr && World->GameState->PlayerArray.Num() > 0){
		for (APlayerState* GameStatePlayer : GetWorld()->GameState->PlayerArray)
		{
			AZanshinPlayerState* CurrentPlayerState = Cast<AZanshinPlayerState>(GameStatePlayer);
			if (CurrentPlayerState != nullptr)
			{
				InPlayerState->Client_AddPlayerWidget(CurrentPlayerState->UserName, CurrentPlayerState->bIsReadyLobby, CurrentPlayerState->TeamNumber);
			}
		}
	}
	else{
		GEngine->AddOnScreenDebugMessage(10, 10.0f, FColor::Green, "Dont Pass check.");
		if (World == nullptr)
		{
			GEngine->AddOnScreenDebugMessage(11, 10.0f, FColor::Green, "World is null.");
		}
	}
}

bool AZanshinPlayerController::CheckReadyState()
{
	UWorld* World = GetWorld();
	bool bBeginTravel = false;
	int32 ReadyPlayers = 0;
	if (World != nullptr && World->GameState->PlayerArray.Num() > 0){
		bBeginTravel = true;
		//Iterate through PlayerStates.
		for (APlayerState* GameStatePlayer : World->GameState->PlayerArray)
		{
			AZanshinPlayerState* CurrentPlayerState = Cast<AZanshinPlayerState>(GameStatePlayer);
			if (CurrentPlayerState != nullptr)
			{
				//Check if they have clicked ready.
				if (!CurrentPlayerState->bIsReadyLobby){
					bBeginTravel = false;
				}
				else{
					ReadyPlayers++;
				}
			}
		}

		AZanshinGameState* CurrentGameState = Cast<AZanshinGameState>(World->GameState);
		CurrentGameState->NumberOfReadyPlayers = ReadyPlayers;
		CurrentGameState->OnRep_ReadyPlayers();

		if (bBeginTravel && World->GameState->PlayerArray.Num() > 0)
		{
			DoServerTravel();
			return true;
		}
		return false;
	}
	else{
		GEngine->AddOnScreenDebugMessage(10, 10.0f, FColor::Green, "Dont Pass check.");
		if (World == nullptr)
		{
			GEngine->AddOnScreenDebugMessage(11, 10.0f, FColor::Green, "World is null.");
		}
		return false;
	}
}

bool AZanshinPlayerController::ServerOnJoinedLobby_Validate(APlayerState* InPlayerState, FName InPlayerName) { return true; }
void AZanshinPlayerController::ServerOnJoinedLobby_Implementation(APlayerState* InPlayerState, FName InPlayerName) { OnJoinedLobby(InPlayerState, InPlayerName); }

void AZanshinPlayerController::Client_SetupInfo_Implementation() { SetupInfo(); }

bool AZanshinPlayerController::Server_SetupInfo_Validate(FName InPlayerName, int32 PlayerTeam) { return true; }
void AZanshinPlayerController::Server_SetupInfo_Implementation(FName InPlayerName, int32 PlayerTeam) {

	AZanshinGameState* ServerGameState = Cast<AZanshinGameState>(GetWorld()->GetGameState());
	AZanshinPlayerState* ServerPlayerState = Cast<AZanshinPlayerState>(PlayerState);
	AZanshinCharacter* ServerCharacter = Cast<AZanshinCharacter>(GetPawn());
	CharacterTeam = PlayerTeam;
	if (ServerGameState != nullptr && ServerPlayerState != nullptr && ServerCharacter != nullptr)
	{
		ServerCharacter->CharacterTeam = PlayerTeam;
		ServerPlayerState->UserName = InPlayerName;
		ServerPlayerState->TeamNumber = PlayerTeam;
		ServerGameState->HandlePlayerSpawn(ServerPlayerState, PlayerTeam);
		
	}
}

void AZanshinPlayerController::RegisterPlayer(int32 InTeamNumber, FName InPlayerName){
	Server_SetupInfo(InPlayerName, InTeamNumber);
}

void AZanshinPlayerController::GetTeamNumber()
{
	UZanshinGameInstance* ZGameInstance = Cast<UZanshinGameInstance>(GetGameInstance());
	check(ZGameInstance);

	if (ZGameInstance != NULL)
	{
		if (ZGameInstance->PlayerTeams.Num() > 0) {
			CharacterTeam = ZGameInstance->PlayerTeams[PlayerState->UniqueId];
			Server_SetupInfo(ZGameInstance->PlayerNames[PlayerState->UniqueId], CharacterTeam);
		} else {
			CharacterTeam = FMath::RandRange(1,2);
		}
	}
}

bool AZanshinPlayerController::Server_SetSpectatorCameraForMap_Validate() { return true; }
void AZanshinPlayerController::Server_SetSpectatorCameraForMap_Implementation() { ClientSetSpectatorCameraForMap(); }

bool AZanshinPlayerController::Server_SetSpectatorCamera_Validate(FVector CameraLocation, FRotator CameraRotation) { return true; }
void AZanshinPlayerController::Server_SetSpectatorCamera_Implementation(FVector CameraLocation, FRotator CameraRotation) {
	ClientSetSpectatorCamera(CameraLocation, CameraRotation);
}

bool AZanshinPlayerController::Server_FindDeathCameraSpot_Validate(FVector CameraLocation, FRotator CameraRotation) { return true; }
void AZanshinPlayerController::Server_FindDeathCameraSpot_Implementation(FVector CameraLocation, FRotator CameraRotation) {
	FindDeathCameraSpot(CameraLocation, CameraRotation);
}

bool AZanshinPlayerController::Server_GoKillerCamera_Validate() { return true; }
void AZanshinPlayerController::Server_GoKillerCamera_Implementation() { 
	ClientGoToKillerCamera();
}

bool AZanshinPlayerController::Client_UpdateMeshVisibility_Validate() { return true; }
void AZanshinPlayerController::Client_UpdateMeshVisibility_Implementation() {
	UpdateMeshVisibility(true);
}

void AZanshinPlayerController::UpdateMeshVisibility(bool bSeeFirstPerson){
	UWorld* World = GetWorld();
	FConstPawnIterator PawnItr = World->GetPawnIterator();
	while(PawnItr){
		AZanshinCharacter* ZanshinCharacter = Cast<AZanshinCharacter>(*PawnItr);
		if (ZanshinCharacter != nullptr)
		{
			USkeletalMeshComponent* FPMesh = ZanshinCharacter->GetSpecifcPawnMesh(true);
			USkeletalMeshComponent* TPMesh = ZanshinCharacter->GetSpecifcPawnMesh(false);
			
			if (FPMesh != nullptr){
				//FPMesh->SetOnlyOwnerSee(bSeeFirstPerson);
				FPMesh->SetHiddenInGame(bSeeFirstPerson, true);
				FPMesh->SetOwnerNoSee(!bSeeFirstPerson);
				//TPMesh->SetVisibility(bSeeFirstPerson, true);
			}
			
			if (TPMesh != nullptr){
				FPMesh->SetHiddenInGame(!bSeeFirstPerson, true);
				TPMesh->SetOwnerNoSee(bSeeFirstPerson);
				//TPMesh->SetVisibility(!bSeeFirstPerson, true);
			}
		}
		++PawnItr;
	}
}

bool AZanshinPlayerController::Server_SetJoinedMap_Validate() { return true; }
void AZanshinPlayerController::Server_SetJoinedMap_Implementation() {
	AZanshinPlayerState* ZPlayerState = Cast<AZanshinPlayerState>(PlayerState);
	ZPlayerState->NotifyJoinedMap();
}

bool AZanshinPlayerController::Client_ShowEndMatchLoadingScreen_Validate(int32 WinningTeam) { 
		return true;
}
void AZanshinPlayerController::Client_ShowEndMatchLoadingScreen_Implementation(int32 WinningTeam) {
	BP_Event_ShowEndOfMatchLoadingScreen(WinningTeam);
}

FZanshinChatMsg AZanshinPlayerController::MakeChat(ChatRecipients MessageType, FString Sender, FString Content){
	FZanshinChatMsg NewMessage;
	NewMessage.Sender = Sender;
	NewMessage.Recipient = MessageType;
	NewMessage.Content = Content;

	return NewMessage;
}

bool AZanshinPlayerController::Server_SetSpectator_Validate() { return true; }
void AZanshinPlayerController::Server_SetSpectator_Implementation() { Spectator(); }