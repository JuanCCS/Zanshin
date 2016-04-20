// Copyright 1998-2015 Epic Games, Inc. All Rights Reserved.

#include "Zanshin.h"
#include "ZanshinPlayerController.h"
#include "ZanshinPlayerState.h"
#include "ZanshinCharacter.h"
#include "AI/ZanshinAIController.h"
#include "Animation/AnimInstance.h"
#include "GameFramework/InputSettings.h"
#include "Networking/NetworkSingleton.h"
#include "Kismet/KismetSystemLibrary.h"

#include "Arrow/ZanshinBasicArrow.h"
#include "Arrow/ZanshinExplosiveArrow.h"
#include "Arrow/ZanshinFireArrow.h"
#include "Arrow/ZanshinTeleportArrow.h"
#include "Arrow/ZanshinTrackerArrow.h"

DEFINE_LOG_CATEGORY_STATIC(LogFPChar, Warning, All);

//For the Editor.
int AZanshinCharacter::EditorTeamNumber = 1;

//////////////////////////////////////////////////////////////////////////
// AZanshinCharacter

AZanshinCharacter::AZanshinCharacter(const class FObjectInitializer& ObjectInitializer)
	:Super(ObjectInitializer)
{
	bReplicates = true;
	bReplicateMovement = true;
	bNetLoadOnClient = true;
	bIsShooting = false;
	bCanCrouch = true;
	bIsReloading = false;
	bCanShoot = true;
	bChangingArrow = false;
	bSelfHeal = false;

	// Set size for collision capsule
	GetCapsuleComponent()->InitCapsuleSize(42.0f, 96.0f);

	//Create a Spring Arm Component
	SpringArmComponent = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraAttachmentArm"));
	SpringArmComponent->bEnableCameraLag = true;
	SpringArmComponent->bEnableCameraRotationLag = true;
	SpringArmComponent->AttachTo(RootComponent);
	SpringArmComponent->TargetArmLength = 0.0f;

	KillerSpringArmComponent = CreateDefaultSubobject<USpringArmComponent>(TEXT("KillerCameraAttachmentArm"));
	KillerSpringArmComponent->bEnableCameraLag = true;
	KillerSpringArmComponent->bEnableCameraRotationLag = true;
	KillerSpringArmComponent->AttachTo(GetMesh());
	KillerSpringArmComponent->TargetArmLength = 0.0f;

	// Create a Camera Component	
	FirstPersonCameraComponent = CreateDefaultSubobject<UCameraComponent>(TEXT("FirstPersonCamera"));
	FirstPersonCameraComponent->AttachTo(SpringArmComponent, USpringArmComponent::SocketName);
	FirstPersonCameraComponent->RelativeLocation = FVector(0, 0, 64.f); // Position the camera
	FirstPersonCameraComponent->bUsePawnControlRotation = true;

	KillerCameraComponent = CreateDefaultSubobject<UCameraComponent>(TEXT("KillerCamera"));
	KillerCameraComponent->AttachTo(KillerSpringArmComponent, USpringArmComponent::SocketName);
	KillerCameraComponent->RelativeLocation = FVector(0.f, 0.f, 0.f); // Position the camera
	KillerCameraComponent->bUsePawnControlRotation = true;

	// Create the Arms Mesh	
	MeshArms = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("FPSHands"));
	MeshArms->AttachTo(FirstPersonCameraComponent);
	MeshArms->CastShadow = false;
	MeshArms->bOnlyOwnerSee = true;
	MeshArms->bOwnerNoSee = false;
	MeshArms->bCastDynamicShadow = false;
	MeshArms->bReceivesDecals = false;
	MeshArms->SetOnlyOwnerSee(true);
	MeshArms->MeshComponentUpdateFlag = EMeshComponentUpdateFlag::OnlyTickPoseWhenRendered;
	MeshArms->PrimaryComponentTick.TickGroup = TG_PrePhysics;
	MeshArms->bChartDistanceFactor = false;
	MeshArms->SetCollisionObjectType(ECC_Pawn);
	MeshArms->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	MeshArms->SetCollisionResponseToAllChannels(ECR_Ignore);

	ArrowSocketComponent = CreateDefaultSubobject<UArrowComponent>(TEXT("ArrowSpawn"));
	ArrowSocketComponent->AttachTo(MeshArms);
	ArrowSocketComponent->RelativeLocation = FVector(0, 30.f, 155.f);
	ArrowSocketComponent->RelativeRotation = FRotator(0.0f, 90.f, 0.f);


	// set our turn rates for input
	MeleeRangeCollider = CreateDefaultSubobject<UBoxComponent>(TEXT("MeleeBoxRange"));
	MeleeRangeCollider->AttachTo(RootComponent);
	MeleeRangeCollider->SetBoxExtent(FVector(50, 50, 40));
	MeleeRangeCollider->SetRelativeLocation(FVector(80, 0, 40));

	// set our arrow audio collider
	ArrowDetection = CreateDefaultSubobject<USphereComponent>(TEXT("ArrowRange"));
	ArrowDetection->AttachTo(RootComponent);
	
	//Assign the properties of the Third Person Model
	GetMesh()->bCastHiddenShadow = true;
	GetMesh()->bOnlyOwnerSee = false;
	GetMesh()->bOwnerNoSee = true;
	GetMesh()->bReceivesDecals = false;
	GetMesh()->SetCollisionObjectType(ECC_Pawn);
	GetMesh()->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	GetMesh()->SetCollisionResponseToChannel(ECC_Visibility, ECR_Block);

	

	// set our turn rates for input
	Health = 100;

	//Create the Particle System Component for the character
	ParticleOnCharacter = CreateDefaultSubobject<UParticleSystemComponent>(TEXT("EffectsParticle"));
	ParticleOnCharacter->AttachTo(GetCapsuleComponent());

	GetCharacterMovement()->SetIsReplicated(true);

	bIsRunning = false;
	MaxWalkSpeedDefault = GetCharacterMovement()->MaxWalkSpeed;
	CurrentStamina = 1.0f;
	StaminaThreshold = 0.3f;
	StaminaConsume = 0.1f;
	changingArrowTimer = 0.8f;
	bIsSwitchingArrows = false;
}

//Initialize Replicated Properties

void AZanshinCharacter::GetLifetimeReplicatedProps(TArray< FLifetimeProperty > & OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(AZanshinCharacter, Health);
	DOREPLIFETIME(AZanshinCharacter, CharacterTeam);
	DOREPLIFETIME(AZanshinCharacter, bIsRunning);
	DOREPLIFETIME(AZanshinCharacter, bIsShooting);
	DOREPLIFETIME(AZanshinCharacter, bCanShoot);
	DOREPLIFETIME(AZanshinCharacter, bChangingArrow);
	DOREPLIFETIME(AZanshinCharacter, bIsSwitchingArrows);
	DOREPLIFETIME(AZanshinCharacter, bIsAiming);
	DOREPLIFETIME(AZanshinCharacter, bIsJetJumping);
	DOREPLIFETIME(AZanshinCharacter, bIsReloading);
	DOREPLIFETIME(AZanshinCharacter, CurrentStamina);
	DOREPLIFETIME(AZanshinCharacter, currentArrowAmmo);
	DOREPLIFETIME(AZanshinCharacter, maxCurrentArrowAmmo);
	DOREPLIFETIME(AZanshinCharacter, bIsSwitchingArrows);
	DOREPLIFETIME(AZanshinCharacter, currentArrow);
	DOREPLIFETIME(AZanshinCharacter, ArrowIndex);
	DOREPLIFETIME(AZanshinCharacter, bSelfHit);
	DOREPLIFETIME(AZanshinCharacter, BaseLookUpRate);
	DOREPLIFETIME(AZanshinCharacter, BaseTurnRate);
	DOREPLIFETIME(AZanshinCharacter, bEnemyHit);
	DOREPLIFETIME(AZanshinCharacter, bIsMeleeAttack);
	DOREPLIFETIME(AZanshinCharacter, bIsNormalKill);
	DOREPLIFETIME(AZanshinCharacter, bIsHeadShotKill);
	DOREPLIFETIME(AZanshinCharacter, bIsMeleeKill);
	DOREPLIFETIME(AZanshinCharacter, SpeedAmount);
	DOREPLIFETIME(AZanshinCharacter, bArrowChanged);
	DOREPLIFETIME(AZanshinCharacter, bIsDead);
	DOREPLIFETIME(AZanshinCharacter, ActualDamageText);
	DOREPLIFETIME(AZanshinCharacter, bSelfHeal);
	DOREPLIFETIME(AZanshinCharacter, bCanDieNextArrow);
	DOREPLIFETIME(AZanshinCharacter, bMeleeHitAttacker);
	DOREPLIFETIME(AZanshinCharacter, bMeleeHitDefender);
	DOREPLIFETIME(AZanshinCharacter, bIsSlowed);
}

//////////////////////////////////////////////////////////////////////////
// Input

void AZanshinCharacter::SetupPlayerInputComponent(class UInputComponent* InputComponent)
{
	// set up gameplay key bindings
	check(InputComponent);

	//////////////////////////////////////////////////////////////////////////
	// Movement

	InputComponent->BindAxis("MoveForward", this, &AZanshinCharacter::MoveForward);
	InputComponent->BindAxis("MoveRight", this, &AZanshinCharacter::MoveRight);

	InputComponent->BindAction("Jump", IE_Pressed, this, &AZanshinCharacter::OnJump);

	InputComponent->BindAction("JetJump", IE_Pressed, this, &AZanshinCharacter::OnJetJump);

	InputComponent->BindAction("Crouch", IE_Pressed, this, &AZanshinCharacter::StartCrouch);
	InputComponent->BindAction("Crouch", IE_Released, this, &AZanshinCharacter::StopCrouch);

	InputComponent->BindAction("Run", IE_Pressed, this, &AZanshinCharacter::Run);
	InputComponent->BindAction("Run", IE_Released, this, &AZanshinCharacter::StopRun);

	//////////////////////////////////////////////////////////////////////////
	// Fire
	InputComponent->BindAction("Fire", IE_Pressed, this, &AZanshinCharacter::Fire);
	//InputComponent->BindAction("Fire", IE_Released, this, &AZanshinCharacter::PullString);

	InputComponent->BindAction("CancelFire", IE_Pressed, this, &AZanshinCharacter::PullString);
	InputComponent->BindAction("CancelFire", IE_Released, this, &AZanshinCharacter::CancelShot);

	//////////////////////////////////////////////////////////////////////////
	// Melee
	InputComponent->BindAction("Melee", IE_Pressed, this, &AZanshinCharacter::MeleeAttack);

	//////////////////////////////////////////////////////////////////////////
	// Change Arrow

	InputComponent->BindAction("PreviousWeapon", IE_Pressed, this, &AZanshinCharacter::PreviousWeapon);
	InputComponent->BindAction("NextWeapon", IE_Pressed, this, &AZanshinCharacter::NextWeapon);

	InputComponent->BindAction("PreviousWeaponScroll", IE_Pressed, this, &AZanshinCharacter::PreviousWeapon);
	InputComponent->BindAction("NextWeaponScroll", IE_Pressed,this, &AZanshinCharacter::NextWeapon);

	InputComponent->BindAction("ArrowOne", IE_Pressed, this, &AZanshinCharacter::ArrowOne);
	InputComponent->BindAction("ArrowTwo", IE_Pressed, this, &AZanshinCharacter::ArrowTwo);
	InputComponent->BindAction("ArrowThree", IE_Pressed, this, &AZanshinCharacter::ArrowThree);
	InputComponent->BindAction("ArrowFour", IE_Pressed, this, &AZanshinCharacter::ArrowFour);
	InputComponent->BindAction("ArrowFive", IE_Pressed, this, &AZanshinCharacter::ArrowFive);

	//////////////////////////////////////////////////////////////////////////
	// Aim

	// We have 2 versions of the rotation bindings to handle different kinds of devices differently
	// "turn" handles devices that provide an absolute delta, such as a mouse.
	// "turnrate" is for devices that we choose to treat as a rate of change, such as an analog joystick
	InputComponent->BindAxis("Turn", this, &AZanshinCharacter::TurnAtRateMouse);
	InputComponent->BindAxis("TurnRate", this, &AZanshinCharacter::TurnAtRate);
	InputComponent->BindAxis("LookUp", this, &AZanshinCharacter::LookUpAtRateMouse);
	InputComponent->BindAxis("LookUpRate", this, &AZanshinCharacter::LookUpAtRate);
}

void AZanshinCharacter::PostInitializeComponents()
{
	Super::PostInitializeComponents();

	SpawnBow();
}

void AZanshinCharacter::BeginPlay()
{
	Super::BeginPlay();

	//GetMesh()->bOwnerNoSee = true;
	//characterIndex = FMath::RandRange(0, 2);
	//Set Melee Range and Dynamic
	MeleeRangeCollider->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	MeleeRangeCollider->OnComponentBeginOverlap.AddDynamic(this, &AZanshinCharacter::OnOverlapBegin);

	ArrowDetection->OnComponentBeginOverlap.AddDynamic(this, &AZanshinCharacter::OnOverlapBeginAudio);

	//Should be done on the PlayerController and passed to the Character on BeginPlay.
	UZanshinGameInstance* ZGameInstance = Cast<UZanshinGameInstance>(GetGameInstance());
	if(ZGameInstance != nullptr)
	{ 
		AZanshinPlayerController* PC = Cast<AZanshinPlayerController>(GetController());
		if (PC != nullptr){
			SetOwner(PC);

			PC->Client_UpdateMeshVisibility();
			HandleColorChange();

			//Assign teams in the editor.
#if WITH_EDITOR
			EditorTeamNumber++;
			PC->CharacterTeam = EditorTeamNumber;
			CharacterTeam = EditorTeamNumber;
			EditorTeamNumber = (EditorTeamNumber % 2) + 1;
#endif
		}
		
		AZanshinAIController* AIController = Cast<AZanshinAIController>(GetController());
		if (AIController != nullptr) {
			AIController->CharacterTeam = CharacterTeam;
		}
	}

	for (TSubclassOf<class AZanshinBasicArrow> Arrow : Arrows)
	{
		FArrowData ArrowsDataObject;
		ArrowsDataObject.MaxAmmo = Arrow.GetDefaultObject()->GetMaxAmmo();
		ArrowsDataObject.AddAmmo(Arrow.GetDefaultObject()->GetInitialAmmo());
		ArrowsData.Add(Arrow.GetDefaultObject()->GetClass()->GetName(), ArrowsDataObject);
	}

	maxArrowNavigation = Arrows.Num() - 1;
	currentArrow = 0;
	currentArrowAmmo = Arrows[0].GetDefaultObject()->GetInitialAmmo();

	CreateAnimationArrow();

	SetCurrentArrowAmmo();
	SetCurrentMaxArrowAmmo();

	bCanJetJump = true;	
	
	UWorld* const World = GetWorld();
	if (World != nullptr) {
		World->GetTimerManager().SetTimer(TimerHandle_ShowFirstArrow, this, &AZanshinCharacter::ShowFirstArrow, 0.25f);
		CharacterAudioManager = AAudioManager::Get(World);
	}

	Client_PlaySound_Attached(CharacterAudioManager->Spawn);

	bAllowJetJump = false;
	bCanDieNextArrow = false;
}

void AZanshinCharacter::SetupInfo()
{
	UZanshinGameInstance* ZGameInstance = (UZanshinGameInstance*)GetGameInstance();
	AZanshinPlayerController* MyPlayerController = Cast<AZanshinPlayerController>(GetController());
	if (MyPlayerController != nullptr && ZGameInstance != nullptr) {
		if (IsFirstPerson())
		{
			MyPlayerController->RegisterPlayer(ZGameInstance->PlayerTeamNumber, ZGameInstance->PlayerName);
		}
	}
}

void AZanshinCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	ControlStamina(DeltaTime);

	if (!bIsShooting)
	{
		if (FirstPersonCameraComponent != nullptr)
		{
			currentCameraFOV = FMath::Lerp(currentCameraFOV, 90.0f, 0.1f);
			FirstPersonCameraComponent->FieldOfView = currentCameraFOV;
		}
	} 
	else 
	{
		currentCameraFOV = FMath::Lerp(currentCameraFOV, 55.0f, 0.025f);
		FirstPersonCameraComponent->FieldOfView = currentCameraFOV;
	}

	if (bIsAiming)
	{
		if (!bIsReloading)
		{
			FastShotTimer += DeltaTime;
			if (FastShotTimer >= FastShotLimit)
			{
				ArrowSocketComponent->RelativeLocation = FVector(0, 30.f, 175.f);
				ArrowSocketComponent->RelativeRotation = FRotator(0.0f, 90.f, 0.f);
			}
		}
	}

	if (!ColorUpdated){
		HandleColorChange();
	}

	FVector forwardVector = ArrowSocketComponent->GetForwardVector() * 55000.0f + ArrowSocketComponent->GetComponentLocation();
	TArray<AActor*> Actors;
	FHitResult OutHit(ForceInit);

	//UKismetSystemLibrary::DrawDebugLine(GetWorld(), ArrowSocketComponent->GetComponentLocation(), forwardVector, FLinearColor::Red, 0.0f, 5.0f);
	UKismetSystemLibrary::LineTraceSingle_NEW(GetWorld(), ArrowSocketComponent->GetComponentLocation(), forwardVector, ETraceTypeQuery::TraceTypeQuery1, true, Actors, EDrawDebugTrace::None, OutHit, true);
	if (Cast<AZanshinCharacter>(OutHit.GetActor()) && OutHit.GetActor() != this && Cast<AZanshinCharacter>(OutHit.GetActor())->CharacterTeam!=CharacterTeam)
	{
		bEnemyOnSight = true;
		bAllyOnSight = false;
	}
	else if (Cast<AZanshinCharacter>(OutHit.GetActor()) && OutHit.GetActor() != this && Cast<AZanshinCharacter>(OutHit.GetActor())->CharacterTeam == CharacterTeam)
	{
		bEnemyOnSight = false;
		bAllyOnSight = true;
	}
	else
	{
		bEnemyOnSight = false;
		bAllyOnSight = false;
	}

	//Health += SelfHeal;

	if (Health <= HearBeatTreshold){
		//Check if the sound has been spawned.
		if (HeartBeatSound != nullptr){
			//Pretty lerp from 0 to 1.
			HeartBeatSound->SetVolumeMultiplier(FMath::InterpEaseIn(HeartBeatSound->VolumeMultiplier, 1.f, DeltaTime, 1.f));
			if (!HeartBeatSound->bIsActive)
				//Play once.
			HeartBeatSound->Play();
		}
	}
	else{
		if (HeartBeatSound != nullptr && HeartBeatSound->bIsActive){
			//Pretty lerp from 1 to 0.
			HeartBeatSound->SetVolumeMultiplier(FMath::InterpEaseIn(HeartBeatSound->VolumeMultiplier, 0.f, DeltaTime, 1.f));
			//Stop it when it reaches 0.
			if (HeartBeatSound->VolumeMultiplier <= 0.f)
				HeartBeatSound->Stop();
		}
	}

	if (Health >= MaxHealth)
	{
		bSelfHit = false;
		Health = MaxHealth;
	}
	if (!bCanJetJump)
	{
		JetJumpTime += DeltaTime;
		if (JetJumpTime > TimeToJetJump)
		{
			bCanJetJump = true;
			JetJumpTime = TimeToJetJump;
		}
	}
}

//////////////////////////////////////////////////////////////////////////
// Ammo

void AZanshinCharacter::AddArrow(TSubclassOf<class AZanshinBasicArrow> Arrow, int32 Ammo)
{
	int32 HUDAmmo = 0;
	if (Role == ROLE_Authority){
		HUDAmmo = ArrowsData[Arrow.GetDefaultObject()->GetClass()->GetName()].AddAmmo(Ammo);
	}

	Client_AddAmmo(Arrow, HUDAmmo);
	Client_PlaySound_Attached(CharacterAudioManager->AmmoPickup);

}
	
void AZanshinCharacter::Client_AddAmmo_Implementation(TSubclassOf<class AZanshinBasicArrow> Arrow, int32 AmmoAmount) { 
	ArrowsData[Arrow.GetDefaultObject()->GetClass()->GetName()].AddAmmo(AmmoAmount);
	if (AmmoAmount != -1)
		BP_Event_AddAmmoToHud(Arrow, AmmoAmount);
}
bool AZanshinCharacter::Client_AddAmmo_Validate(TSubclassOf<class AZanshinBasicArrow> Arrow, int32 AmmoAmount) { return true; }

void AZanshinCharacter::SetCurrentArrowAmmo()
{
	if (Role < ROLE_Authority){
		Server_SetCurrentAmmo();
		return;
	}

	//currentArrowAmmo = Bow->GetCurrentAmmo();
	currentArrowAmmo = ArrowsData[Arrows[GetArrowIndex()].GetDefaultObject()->GetClass()->GetName()].CurrentAmmo;
}

void AZanshinCharacter::SetCurrentMaxArrowAmmo()
{
	if (Role < ROLE_Authority){
		Server_SetCurrentMaxAmmo();
		return;
	}

	//maxCurrentArrowAmmo = Bow->GetMaxCurrentAmmo();
	maxCurrentArrowAmmo = ArrowsData[Arrows[GetArrowIndex()].GetDefaultObject()->GetClass()->GetName()].MaxAmmo;
}

int32 AZanshinCharacter::SendArrowsAmmo(int32 ArrowIndex)
{
	int32 ammoToReturn = ArrowsData[Arrows[ArrowIndex].GetDefaultObject()->GetClass()->GetName()].CurrentAmmo;

	return ammoToReturn;

}

//////////////////////////////////////////////////////////////////////////
// Fire

//Run what is Important to the client outside of the server function.
void AZanshinCharacter::PullString()
{
	//FirstPersonCameraComponent->FieldOfView = currentCameraFOV;
	//New Controls
	bIsReloading = false;

	if (currentArrowAmmo > 0 && bIsReloading == false && bIsMeleeAttack == false)
	{
		Client_PlaySound_Attached(CharacterAudioManager->PullString);
		UpdateHUDIndicators(true);
		ServerPullString();	
	}
}

void AZanshinCharacter::OnPullString()
{
	if (Bow != nullptr)
	{
		if (!bIsShooting)
			GetCharacterMovement()->MaxWalkSpeed = ShootWalkSpeed;

		Bow->OnPullString();
		bIsShooting = true;
		bCanCrouch = false;
		bIsAiming = true;
		bIsRunning = false;
		//BaseTurnRate *= 0.5f;
	}
}

//Cancels the shot
void AZanshinCharacter::CancelShot()
{
	if (Role < ROLE_Authority){
		Server_CancelShot();
		return;
	}

	if (bIsShooting)
	{
		bIsCancelling = true;
		
		Bow->ResetPower();
		
		bIsShooting = false;
		bCanCrouch = true;
		bIsAiming = false;

		GetCharacterMovement()->MaxWalkSpeed = MaxWalkSpeedDefault;
		ResetArrowSocket();
		
		//AfterFire();
	}
}

//Runs on Client
void AZanshinCharacter::Fire()
{
	if (!bIsReloading && currentArrowAmmo > 0)//Was bCanShoot
	{
		if (Role < ROLE_Authority){
			Client_PlaySound_Attached(CharacterAudioManager->ArrowShot);
			Client_AddAmmo(Arrows[currentArrow], -1);
		}
		ServerFire(ArrowSocketComponent->GetComponentLocation(), ArrowSocketComponent->GetComponentRotation());
		UpdateHUDIndicators(false);
	}
}

//Runs on Server
void AZanshinCharacter::OnFire(FVector ClientSpawnLocation, FRotator ClientSpawnRotation)
{
	if (Bow != nullptr)
	{
		if (AnimationArrow != nullptr) {
			AnimationArrow->Destroy();
		}

		Bow->OnFire(ClientSpawnLocation, ClientSpawnRotation);
		SetCurrentArrowAmmo();

		ChangeToDefaultArrowIfNoAmmo();

		AfterFire();

		Client_PlaySound_Attached(CharacterAudioManager->ArrowShot);
		bCanShoot = false;
		//BaseTurnRate *= 2.f;

		AZanshinPlayerState* ZPlayerState = Cast<AZanshinPlayerState>(PlayerState);
		if (ZPlayerState) {
			ZPlayerState->ZanshinPlayerData.ArrowsFired++;
		}
	}
}

void AZanshinCharacter::ChangeToDefaultArrowIfNoAmmo()
{
	if (currentArrowAmmo <= 0)
	{
		currentArrow = ArrowIndex = -1;
		bIsAiming = false;

		NextWeapon();
	}
}

void AZanshinCharacter::CheckIfTheNextArrowHasAmmo(int32 Direction)
{
	if (ArrowsData[Arrows[ArrowIndex].GetDefaultObject()->GetClass()->GetName()].CurrentAmmo <= 0)
	{
		ChangeArrowAmmoZeroIndex++;
		if (ChangeArrowAmmoZeroIndex >= ArrowsData.Num())
		{
			ChangeArrowAmmoZeroIndex = 0;
			currentArrow = ArrowIndex = 0;
			return;
		}

		currentArrow = ArrowIndex += Direction;

		if (ArrowIndex < 0 || ArrowIndex >= ArrowsData.Num())
		{
			if (Direction > 0) {
				currentArrow = ArrowIndex = 0;
			} else {
				currentArrow = ArrowIndex = ArrowsData.Num() - 1;
			}
		}

		CheckIfTheNextArrowHasAmmo(Direction);
	}

	ChangeArrowAmmoZeroIndex = 0;
}

//Returns the camera zoom, and enable new shots
void AZanshinCharacter::AfterFire()
{
	bIsShooting = false;
	bCanCrouch = true;
	bIsAiming = false;
	bIsReloading = true;
	bCanShoot = true;
	GetCharacterMovement()->MaxWalkSpeed = MaxWalkSpeedDefault;
	ResetArrowSocket();
	UWorld* World = GetWorld();
	World->GetTimerManager().SetTimer(ReloadTimerHandle, this, &AZanshinCharacter::SetCanShoot, .75f);
}

void AZanshinCharacter::SetCanShoot() { bCanShoot = true; }

void AZanshinCharacter::ResetArrowSocket()
{
	FastShotTimer = 0.0f;
	ArrowSocketComponent->RelativeLocation = FVector(0, 30.f, 155.f);
	ArrowSocketComponent->RelativeRotation = FRotator(0.0f, 90.f, 0.f);
}

void AZanshinCharacter::ChangeCameraActive(bool bIsFirstPerson)
{
	if (bIsFirstPerson) {
		FirstPersonCameraComponent->bIsActive = true;
		FirstPersonCameraComponent->SetActive(true);
		FirstPersonCameraComponent->Activate();

		KillerCameraComponent->bIsActive = false;
		KillerCameraComponent->SetActive(false);
		KillerCameraComponent->Deactivate();
	}
	else {
		FirstPersonCameraComponent->bIsActive = false;
		FirstPersonCameraComponent->SetActive(false);
		FirstPersonCameraComponent->Deactivate();

		KillerCameraComponent->bIsActive = true;
		KillerCameraComponent->SetActive(true);
		KillerCameraComponent->Activate();
	}
}

//////////////////////////////////////////////////////////////////////////
// Melee

void AZanshinCharacter::MeleeAttack()
{
	if (Role < ROLE_Authority){
		Server_MeleeAttack();
		return;
	}

	if (!bIsMeleeAttack)
	{
		bIsMeleeAttack = true;

		MeleeRangeCollider->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
		Client_PlaySound_Attached(CharacterAudioManager->MeleeMiss);
	}

	//REMOVED SERGIO Now the animation blueprint control when to set the melee attack false.
	UWorld* World = GetWorld();
	World->GetTimerManager().SetTimer(MeleeTimerHandle, this, &AZanshinCharacter::NoMeleeCollision, 0.5f);
}


void AZanshinCharacter::NoMeleeCollision()
{
	MeleeRangeCollider->SetCollisionEnabled(ECollisionEnabled::NoCollision);
}

//////////////////////////////////////////////////////////////////////////
// Check Melee Collision Box

void AZanshinCharacter::OnOverlapBegin(class AActor* OtherActor, class UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	FPointDamageEvent DamageEvent;
	AZanshinCharacter* Enemy = Cast<AZanshinCharacter>(OtherActor);
	AZanshinPlayerController* PC = Cast<AZanshinPlayerController>(GetController());

	if (Enemy != nullptr) {
		AZanshinPlayerController* PCEnemy = Cast<AZanshinPlayerController>(Enemy->GetController());

		if (Enemy != this &&
			PC != nullptr && PCEnemy != nullptr &&
			PC->CharacterTeam != PCEnemy->CharacterTeam)
		{
			FPointDamageEvent DamageEvent;
			Enemy->CurrentKillType = KillType::MELEE;
			Enemy->TakeDamage(25.0f, DamageEvent, GetController(), this);
			bIsMeleeKill = true;
			bMeleeHitAttacker = true;
			Client_PlaySound_Attached(CharacterAudioManager->MeleeHit);
			BP_Event_ShowFloatDamageForMelee(Enemy, 25.0f);
		}
	}
}

void AZanshinCharacter::OnOverlapBeginAudio(class AActor* OtherActor, class UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	AZanshinBasicArrow* Arrow = Cast<AZanshinBasicArrow>(OtherActor);

	if (Arrow != nullptr && Arrow->GetMyPawn()!=this && Arrow->GetVelocity()!=FVector::ZeroVector)
	{
		Client_PlaySound_Attached(CharacterAudioManager->ArrowSwoosh);
	}
}

//////////////////////////////////////////////////////////////////////////
// Display Score


//////////////////////////////////////////////////////////////////////////
// Movement

void AZanshinCharacter::MoveForward(float Value)
{
	if (Value != 0.0f) {
		// add movement in that direction
		AddMovementInput(GetActorForwardVector(), Value);

		bIsMovingForward = true;
	} else {
		bIsMovingForward = false;
	}
}

void AZanshinCharacter::MoveRight(float Value)
{
	if (Value != 0.0f) {
		// add movement in that direction
		AddMovementInput(GetActorRightVector(), Value);

		bIsMovingRight = true;
	} else {
		bIsMovingRight = false;
	}
}

//////////////////////////////////////////////////////////////////////////
// Run

void AZanshinCharacter::ControlStamina(float DeltaTime)
{
	(bIsRunning && (bIsMovingForward || bIsMovingRight) && !GetCharacterMovement()->IsFalling())
		? CurrentStamina -= (StaminaConsume * DeltaTime)
		: CurrentStamina += (StaminaConsume * DeltaTime) / 2;
	
	CurrentStamina = FMath::Clamp(CurrentStamina, 0.0f, MaxStamina);

	if (CurrentStamina <= 0) {
		CurrentStamina = 0;
		Run();
	}
}

void AZanshinCharacter::Run()
{
	if (Role < ROLE_Authority){
		ServerRun();
	}

	if (!bIsAiming) {
		bIsRunning = !bIsRunning;
	}

	if (bIsRunning) {
		if (GetCharacterMovement()->MaxWalkSpeed != RunnigSpeed)
			GetCharacterMovement()->MaxWalkSpeed = RunnigSpeed; 
	}
	else {
		if (GetCharacterMovement()->MaxWalkSpeed != MaxWalkSpeedDefault)
			GetCharacterMovement()->MaxWalkSpeed = MaxWalkSpeedDefault;
	}
}

void AZanshinCharacter::StopRun()
{
	if (Role < ROLE_Authority){
		ServerStopRun();
	}
	bIsRunning = false;
	if (GetCharacterMovement()->MaxWalkSpeed != MaxWalkSpeedDefault)
		GetCharacterMovement()->MaxWalkSpeed = MaxWalkSpeedDefault;
}

bool AZanshinCharacter::IsRunning() const
{	
	if (!GetCharacterMovement()) {
		return false;
	}

	return !GetVelocity().IsZero() && (GetVelocity().GetSafeNormal2D() | GetActorForwardVector()) > -0.1;
}

bool AZanshinCharacter::IsbRunning() const { return bIsRunning; }

void AZanshinCharacter::StartCrouch() 
{ 
	if (bCanCrouch)
	{
		Crouch(true);
	}
}

void AZanshinCharacter::StopCrouch() { UnCrouch(true); }

void AZanshinCharacter::SlowDown(float Amount)
{
	SpeedAmount = Amount;
	bIsSlowed = true;
	GetCharacterMovement()->MaxWalkSpeed = MaxWalkSpeedDefault*SpeedAmount;

	GetWorldTimerManager().SetTimer(TimerHandle_GoBackToNormalSpeed, this, &AZanshinCharacter::BackToNormalSpeed, TimeBackNormalSpeed);
}

void AZanshinCharacter::BackToNormalSpeed()
{
	SpeedAmount = 1.0f;
	bIsSlowed = false;
	GetCharacterMovement()->MaxWalkSpeed = MaxWalkSpeedDefault;
}

//////////////////////////////////////////////////////////////////////////
// Change Arrow 

void AZanshinCharacter::PreviousWeapon() 
{	
	if (!bIsAiming)
	{
		if (Role < ROLE_Authority)
		{
			ServerPreviousArrow();
		}
		else
		{
			//if (!bIsSwitchingArrows)
			//{
				bIsSwitchingArrows = true;
				bChangingArrow = true;

				ArrowChangedToHUDTrue();

				Bow->PreviousWeapon();

				ArrowIndex--;

				currentArrow = GetArrowIndex();

				Client_PlaySound_Attached(CharacterAudioManager->ChangingArrowHUD);

				Bow->BP_Event_UpdateString(ArrowIndex);

				CheckIfTheNextArrowHasAmmo(-1);

				SetCurrentArrowAmmo();
				SetCurrentMaxArrowAmmo();

				//GetWorldTimerManager().SetTimer(TimerHandle_DestroyComponentsAttaches, this, &AZanshinCharacter::RestrainSwitching, changingArrowTimer);

				bIsReloading = true;
			//}
		}
	}
}

void AZanshinCharacter::NextWeapon() 
{
	if (!bIsAiming)
	{
		if (Role < ROLE_Authority)
		{
			ServerNextArrow();
		}
		else
		{
			//if (!bIsSwitchingArrows)
			//{
				bIsSwitchingArrows = true;
				bChangingArrow = true;

				ArrowChangedToHUDTrue();

				Bow->NextWeapon();
				
				ArrowIndex++;

				currentArrow = GetArrowIndex();

				Client_PlaySound_Attached(CharacterAudioManager->ChangingArrowHUD);

				Bow->BP_Event_UpdateString(ArrowIndex);

				CheckIfTheNextArrowHasAmmo(1);

				SetCurrentArrowAmmo();
				SetCurrentMaxArrowAmmo();

				//GetWorldTimerManager().SetTimer(TimerHandle_DestroyComponentsAttaches, this, &AZanshinCharacter::RestrainSwitching, changingArrowTimer);

				bIsReloading = true;
			//}
		}
	}
}

void AZanshinCharacter::RestrainSwitching() {
	bIsSwitchingArrows = false; 			
	bChangingArrow = false;
}

void AZanshinCharacter::ArrowOne() { ChangeArrowFromKey(0); }
void AZanshinCharacter::ArrowTwo() { ChangeArrowFromKey(1); }
void AZanshinCharacter::ArrowThree() { ChangeArrowFromKey(2); }
void AZanshinCharacter::ArrowFour() { ChangeArrowFromKey(3); }
void AZanshinCharacter::ArrowFive() { ChangeArrowFromKey(4); }

void AZanshinCharacter::ChangeArrowFromKey(int32 ArrowIndexKey)
{
	if (Role < ROLE_Authority) {
		Server_ChangeArrowFromKey(ArrowIndexKey);
	} else {
		if (ArrowIndexKey != ArrowIndex) {
			ArrowIndex = (ArrowIndexKey + 1);
			currentArrow = GetArrowIndex();

			PreviousWeapon();
		}
	}
}

void AZanshinCharacter::ShowFirstArrow()
{
	currentArrow = ArrowIndex = 4;

	if (Role < ROLE_Authority) {
		Server_ChangeArrowFromKey(0);
	}
	else {
		ChangeArrowFromKey(0);
	}

	currentArrow = ArrowIndex = 0;
}

//////////////////////////////////////////////////////////////////////////
// Set Particle on Character
void AZanshinCharacter::SetParticle(UParticleSystem* ParticleFromArrow) 
{ 
	ParticleOnCharacter->SetTemplate(ParticleFromArrow);
	ParticleOnCharacter->SetVisibility(true);
	ParticleOnCharacter->bOwnerNoSee = true;
	UWorld* const World = GetWorld();
	if (World != nullptr) 
	{
		World->GetTimerManager().SetTimer(TimerHandle_EraseParticle, this, &AZanshinCharacter::EraseParticle, TimeToResetParticle);
	}
}

void AZanshinCharacter::EraseParticle()
{
	ParticleOnCharacter->SetVisibility(false);
}

void AZanshinCharacter::ArrowChangedToHUDTrue() { 
	if (Role < ROLE_Authority){
		Server_SetArrowChanged();
		return;
	}
	bArrowChanged = true;
}

bool AZanshinCharacter::Server_SetArrowChanged_Validate() {
	return true;
}

void AZanshinCharacter::Server_SetArrowChanged_Implementation() {
	ArrowChangedToHUDTrue();
}


void AZanshinCharacter::ArrowChangedToHUDFalse()
{	
	if (Role < ROLE_Authority){
		Server_ResetArrowChanged();
		return;
	}
	bArrowChanged = false;
}

bool AZanshinCharacter::Server_ResetArrowChanged_Validate() {
	return true;
}

void AZanshinCharacter::Server_ResetArrowChanged_Implementation() {
	ArrowChangedToHUDFalse();
}

//////////////////////////////////////////////////////////////////////////
// Jump

void AZanshinCharacter::OnJump()
{
	ServerOnJump();
	Jump();

  	if (bAllowJetJump) {
		OnJetJump();
	}

	bAllowJetJump = true;

	UWorld* const World = GetWorld();
	if (World != nullptr) {
		World->GetTimerManager().SetTimer(TimerHandle_AllowToJetJump, this, &AZanshinCharacter::CancelAllowJetJump, TimeToAllowJetJump);
	}
}

void AZanshinCharacter::OnJetJump()
{
	if (bCanJetJump)
	{
		Client_PlaySound_Attached(CharacterAudioManager->JetJump);

		//Use for jet jump.
		Server_OnJetJump();
		//Jump();

		bCanJetJump = false;
		JetJumpTime = 0.0f;
	}
}

void AZanshinCharacter::CancelAllowJetJump() { bAllowJetJump = false; }

//////////////////////////////////////////////////////////////////////////
// Aim Mouse

void AZanshinCharacter::TurnAtRateMouse(float Rate)
{
	// calculate delta for this frame from the rate information
	AddControllerYawInput(Rate * BaseTurnRate * GetWorld()->GetDeltaSeconds());
}

void AZanshinCharacter::LookUpAtRateMouse(float Rate)
{
	// calculate delta for this frame from the rate information
	AddControllerPitchInput(Rate * BaseLookUpRate * GetWorld()->GetDeltaSeconds());
}

//////////////////////////////////////////////////////////////////////////
// Aim Controller

void AZanshinCharacter::TurnAtRate(float Rate)
{
	// calculate delta for this frame from the rate information
	AddControllerYawInput(Rate * BaseTurnRate * GetWorld()->GetDeltaSeconds());
}

void AZanshinCharacter::LookUpAtRate(float Rate)
{
	// calculate delta for this frame from the rate information
	AddControllerPitchInput(Rate * BaseLookUpRate * GetWorld()->GetDeltaSeconds());
}

FRotator AZanshinCharacter::GetAimOffsets() const
{
	const FVector AimDirWS = GetBaseAimRotation().Vector();
	const FVector AimDirLS = ActorToWorld().InverseTransformVectorNoScale(AimDirWS);
	const FRotator AimRotLS = AimDirLS.Rotation();

	return AimRotLS;
}

//////////////////////////////////////////////////////////////////////////
// Damage

void AZanshinCharacter::AddHealth(int16 Amount)
{
	Health += Amount;
	Health = FMath::Clamp<int16>(Health, 0, MaxHealth);
	bSelfHeal = true;
	Client_PlaySound_Attached(CharacterAudioManager->Healing[characterIndex]);
	Client_PlaySound_Attached(CharacterAudioManager->HealthPickup);
	//Client_PlaySound_Attached(CharacterAudioManager->Healing[characterIndex], GetActorLocation(), 1.f);
}

void AZanshinCharacter::OnShot(class AActor* OtherActor, class UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit)
{
	AZanshinBasicArrow* Arrow = Cast<AZanshinBasicArrow>(OtherActor);

	//Check for an enemy arrow
	if (Arrow != nullptr && CheckToSeeIfItsEnemy(Arrow))
	{
		UWorld* Server_World = GetWorld();

		/*ULISES*/
		//Impact information to Apply Impulse Impact
		FRotator OtherActorRotation = OtherActor->GetActorRotation();
		const FVector OtherActorOrientation = OtherActorRotation.Vector();

		UPrimitiveComponent* ThirdPerson = Hit.GetComponent();
		
		AZanshinPlayerState* EnemyPlayerState = Cast<AZanshinPlayerState>(Arrow->GetMyPawn()->PlayerState);

		AZanshinCharacter* Enemy = Arrow->GetMyPawn();
		if (Enemy != nullptr && EnemyPlayerState != nullptr){
			if (EnemyPlayerState)
			{
				EnemyPlayerState->ZanshinPlayerData.UpdateBodyPartHit(Hit.BoneName);
			}
			if (Hit.BoneName == "head")
			{
				CurrentKillType = KillType::HEADSHOT;

				Enemy->Client_PlaySound_Attached(CharacterAudioManager->HeadShots[Enemy->characterIndex]);
				Enemy->Client_PlaySound_Attached(CharacterAudioManager->HeadShotFeedBack);
				Client_PlaySound_Attached(CharacterAudioManager->HeadShot);

				DealDamage(Health, Arrow);
			}
			else if (Hit.BoneName == "spine_03" || Hit.BoneName == "spine_02" || Hit.BoneName == "spine_01")
			{
				CurrentKillType = KillType::NORMAL;

				Enemy->Client_PlaySound_Attached(CharacterAudioManager->ChestShot);
				Client_PlaySound_Attached(CharacterAudioManager->ChestShot);

				DealDamage(Arrow->GetDamage(), Arrow);
			}

			else
			{
				CurrentKillType = KillType::NORMAL;

				Enemy->Client_PlaySound_Attached(CharacterAudioManager->ChestShot);
				Client_PlaySound_Attached(CharacterAudioManager->ChestShot);

				float ActualDamage = (Arrow->GetDamage() * PercentDamageArmsAndLegs) / 100;

				DealDamage(ActualDamage, Arrow);
			}

			if (Arrow->KillTypeArrow == ArrowKillType::TRACKER) 
			{
				ActiveNextArrowKill();
			}
			if (Server_World != nullptr) {
				UGameplayStatics::SpawnEmitterAtLocation(Server_World, BloodParticle, Hit.ImpactPoint, Hit.ImpactPoint.Rotation(), false);
			}

			if (Health <= 0.f) {
				NetMulticast_ImpulseOnBone(PokeStrength, Hit, OtherActorOrientation);
			}
			else {
				KillerAssist.AddUnique(Arrow->GetMyPawn());
			}
		}
	}	
}

void AZanshinCharacter::OnHeadShot(class AActor* OtherActor, class UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit)
{
	AZanshinBasicArrow* Arrow = Cast<AZanshinBasicArrow>(OtherActor);
	
	
	if (Arrow != nullptr && CheckToSeeIfItsEnemy(Arrow)) 
	{
		CurrentKillType = KillType::HEADSHOT;

		DealDamage(Health, Arrow);

		UWorld* Server_World = GetWorld();
		if (Server_World != nullptr){
			UGameplayStatics::SpawnEmitterAtLocation(Server_World, BloodParticle, Hit.ImpactPoint, Hit.ImpactPoint.Rotation(), false);
		}
	}
}

void AZanshinCharacter::OnChestShot(class AActor* OtherActor, class UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit)
{
	AZanshinBasicArrow* Arrow = Cast<AZanshinBasicArrow>(OtherActor);
	

	if (Arrow != nullptr && CheckToSeeIfItsEnemy(Arrow)) 
	{
		CurrentKillType = KillType::NORMAL;

		DealDamage(Arrow->GetDamage(), Arrow);
		UWorld* Server_World = GetWorld();
		if (Server_World != nullptr){
			UGameplayStatics::SpawnEmitterAtLocation(Server_World, BloodParticle, Hit.ImpactPoint, Hit.ImpactPoint.Rotation(), false);
		}
		if (ChestSound != nullptr) 
		{
			//UGameplayStatics::PlaySoundAtLocation(this, ChestSound, GetActorLocation());
			Client_PlaySound(ChestSound);

		}
	}
}

void AZanshinCharacter::OnArmsAndLegsShot(class AActor* OtherActor, class UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit)
{
	AZanshinBasicArrow* Arrow = Cast<AZanshinBasicArrow>(OtherActor);
	

	if (Arrow != nullptr && CheckToSeeIfItsEnemy(Arrow)) 
	{
		CurrentKillType = KillType::NORMAL;

		float ActualDamage = (Arrow->GetDamage() * PercentDamageArmsAndLegs) / 100;

		Client_PlaySound(ChestSound);

		DealDamage(ActualDamage, Arrow);
		UWorld* Server_World = GetWorld();
		if (Server_World != nullptr){
			UGameplayStatics::SpawnEmitterAtLocation(Server_World, BloodParticle, Hit.ImpactPoint, Hit.ImpactPoint.Rotation(), false);

		}

	}
}

void AZanshinCharacter::DealDamage(float Damage, class AZanshinBasicArrow* Arrow)
{
	if (Arrow !=nullptr)
		DirectionOfTheArrow = Arrow->GetActorRotation().Vector();

	if (bCanDieNextArrow) {
		Damage = Health;
	}

	FPointDamageEvent DamageEvent;
	DamageEvent.Damage = Damage;
	CurrentArrowKillType = Arrow->KillTypeArrow;

	if (Cast<AZanshinExplosiveArrow>(Arrow))
		CurrentKillType = KillType::NORMAL;

	TakeDamage(DamageEvent.Damage, DamageEvent, Arrow->GetMyPawn()->GetController(), Arrow->GetMyPawn());

	/*if (Arrow != NULL)
		ArrowStuckOnYou.Add(Arrow);*/
}

void AZanshinCharacter::ActiveNextArrowKill() 
{ 
	bCanDieNextArrow = true;  
	Client_PlaySound_Attached(CharacterAudioManager->Tracked);
}
void AZanshinCharacter::DeactivatedNextArrowKill() 
{ 
	bCanDieNextArrow = false; 
}

float AZanshinCharacter::TakeDamage(float Damage, struct FDamageEvent const& DamageEvent, class AController* EventInstigator, class AActor* DamageCauser)
{
	if (Health <= 0.f) {
		return 0.f;
	}

	AddPlayerFeedback_ReceiveHit();

	const float ActualDamage = Super::TakeDamage(Damage, DamageEvent, EventInstigator, DamageCauser);

	if (ActualDamage > 0.f) {
		Health -= ActualDamage;

		CheckHealth();
		ActualDamageText = ActualDamage;

		AZanshinCharacter* Enemy = Cast<AZanshinCharacter>(DamageCauser);

		if (Enemy != nullptr)
		{
			if (Health <= 0)
			{
				if (CurrentKillType == KillType::HEADSHOT)
				{
				}
				else
				{
				}
				Die(ActualDamage, DamageEvent, EventInstigator, DamageCauser);
			}
		
		}

		MakeNoise(1.0f, EventInstigator ? EventInstigator->GetPawn() : this);
	}

	return ActualDamage;
}

void AZanshinCharacter::CheckHealth()
{
	if (Health < 25)
	{
		Client_PlaySound_Attached(CharacterAudioManager->HeartBeat);
	}
}

bool AZanshinCharacter::Die(float KillingDamage, FDamageEvent const& DamageEvent, AController* Killer, AActor* DamageCauser)
{
	Health = FMath::Min(0.0f, Health);

	//bReplicateMovement = false;
	//HeartBeat->VolumeMultiplier = 0.0f;

	AController* const KilledPlayer = (Controller != nullptr) ? Controller : Cast<AController>(GetOwner());
	if (Role == ROLE_Authority) {

		if (Killer != nullptr)
		{
			AZanshinCharacter* Enemy = Cast<AZanshinCharacter>(Killer->GetPawn());
			if (Enemy != nullptr)
			{
				for (int32 Index = 0; Index != KillerAssist.Num(); ++Index)
				{
					if (KillerAssist[Index] == Enemy) {
						KillerAssist.RemoveAt(Index);
						break;
					}
				}
			}

			GetWorld()->GetAuthGameMode<AZanshinGameMode>()->Killed(Killer, KilledPlayer, this, NULL, CurrentKillType, CurrentArrowKillType, KillerAssist);

			AZanshinCharacter* KillerCharacter = Cast<AZanshinCharacter>(Killer->GetPawn());

			KillerCharacter->Client_UpdateKillType(CurrentKillType);

			AZanshinPlayerController* PC = Cast<AZanshinPlayerController>(GetController());
			if (PC != nullptr) {				
				PC->KilledBy(Cast<AZanshinCharacter>(Killer->GetPawn()));
			}

			DetachFromControllerPendingDestroy();
		}
	}

	SetLifeSpan(LifeSpan);
	
	bIsDead = true;

	if (GetMesh()->GetPhysicsAsset()) 
	{ 
		ActiveRagdoll(); 
	}	

	UWorld* const World = GetWorld();
	if (World != nullptr) {
		World->GetTimerManager().SetTimer(TimerHandle_DestroyComponentsAttaches, this, &AZanshinCharacter::DestroyArrowsStuckOnYou, 3.0f);
	}

	//Add all feedback to the player here. Client Function.
	AddPlayerFeedBack_Death();

	return true;
}

/*ULISES*/
//Take a Hit on the selected Bone.
void AZanshinCharacter::ImpulseOnBone( float ImpulseApplied, const FHitResult& Hit, const FVector ArrowDirection)
{
	const FName BoneName = Hit.BoneName;
	FVector Damage = ArrowDirection * ImpulseApplied;
	GetMesh()->AddImpulseAtLocation(Damage, GetActorLocation(), BoneName);
}

bool AZanshinCharacter::CheckToSeeIfItsEnemy(class AZanshinBasicArrow* Arrow)
{
	if (Arrow  != nullptr && CharacterTeam == Arrow->GetMyPawn()->GetTeam()) {
		Arrow->Destroy();
		return false;
	}

	return true;
}

void AZanshinCharacter::DestroyArrowsStuckOnYou()
{
	if (Role < ROLE_Authority){
		return;
	}

	Bow->Destroy();

	//auto FilteredArray = ArrowStuckOnYou.FilterByPredicate([](const AZanshinBasicArrow* Ptr){
	//	return Ptr != nullptr;
	//});

	//for (AZanshinBasicArrow* Arrow : FilteredArray) {
	//	TArray<USceneComponent*> Childrens;
	//	if (IsValid(Arrow) && Arrow != nullptr){
	//		Arrow->GetRootComponent()->GetChildrenComponents(true, Childrens);
	//		auto ChildrenArray = Childrens.FilterByPredicate([](const USceneComponent* Ptr){
	//			return Ptr != nullptr;
	//		});
	//		/*if (ChildrenArray.Num() > 0)
	//		{
	//			for (auto& Actor : Childrens) {
	//				if (Actor != nullptr)
	//					Actor->DestroyComponent();
	//			}
	//		}
	//		Arrow->SetLifeSpan(0.05f);*/
	//	}
	//}

	bRagdollDisabled = true;
	NetMulticast_DisableRagdoll();
}

//////////////////////////////////////////////////////////////////////////
// Ragdoll

void AZanshinCharacter::ActiveRagdoll()
{
	//NetWork Ragdoll.
	if (Role == ROLE_Authority)
	{
		GetMesh()->SetAllBodiesSimulatePhysics(true);
		GetMesh()->WakeAllRigidBodies();
		GetMesh()->bBlendPhysics = true;
		NetMulticastHandleRagdollEffect();
		NetMulticastAddForceToRagdoll(DirectionOfTheArrow);
	}
}

bool AZanshinCharacter::NetMulticast_DisableRagdoll_Validate() { return true; }
void AZanshinCharacter::NetMulticast_DisableRagdoll_Implementation() { DisableRagdoll(); }

void AZanshinCharacter::DisableRagdoll() 
{ 
	bRagdollDisabled = true;
	GetMesh()->SetCollisionProfileName(TEXT("Ragdoll"));
}

void AZanshinCharacter::AddForceToRagdoll(FVector Direction)
{
	DirectionOfTheArrow = Direction * ForceToRagdoll;
	GetMesh()->AddImpulseAtLocation(DirectionOfTheArrow, GetActorLocation(), "b_head");
}

//////////////////////////////////////////////////////////////////////////
// Getter and Setter

class AZanshinBow* AZanshinCharacter::GetBow() const { return Bow; }

int32 AZanshinCharacter::GetTeam() const { return CharacterTeam; }

bool AZanshinCharacter::IsAiming() const { return bIsAiming; }
void AZanshinCharacter::SetIsAiming(bool NewIsAiming) { bIsAiming = NewIsAiming; }

bool AZanshinCharacter::IsReloading() const { return bIsReloading; }
void AZanshinCharacter::SetIsReloading(bool NewIsReloading) 
{
	if (Role < ROLE_Authority){
		ServerSetIsReloading(NewIsReloading);
		return;
	}

	bIsReloading = NewIsReloading;
}

bool AZanshinCharacter::IsChangingArrow() const { return bChangingArrow; }
void AZanshinCharacter::SetIsChangingArrow(bool NewIsChangingArrow)
{
	if (Role < ROLE_Authority){
		ServerSetIsChangingArrow(NewIsChangingArrow);
		return;
	}

	bChangingArrow = NewIsChangingArrow;
}

bool AZanshinCharacter::IsMeleeAttack() const { return bIsMeleeAttack; }
void AZanshinCharacter::SetIsMeleeAttack(bool NewIsMeleeAttack) 
{ 
	if (Role < ROLE_Authority){
		Server_SetIsMeleeAttack(NewIsMeleeAttack);
		return;
	}
	bMeleeHitAttacker = false;
	bIsMeleeAttack = NewIsMeleeAttack; 
	MeleeRangeCollider->SetCollisionEnabled(ECollisionEnabled::NoCollision);
}
void AZanshinCharacter::Server_SetIsMeleeAttack_Implementation(bool NewIsMeleeAttack) { SetIsMeleeAttack(NewIsMeleeAttack); }
bool AZanshinCharacter::Server_SetIsMeleeAttack_Validate(bool NewIsMeleeAttack) { return true; }

FName AZanshinCharacter::GetFirstPersonBowAttachPoint() const { return FirstPersonBowAttachPoint; }
FName AZanshinCharacter::GetThirdPersonBowAttachPoint() const { return ThirdPersonBowAttachPoint; }

bool AZanshinCharacter::IsCancelling() const{ return bIsCancelling; }
void AZanshinCharacter::SetIsCancelling(bool NewIsCancelling) { bIsCancelling = NewIsCancelling; }

//////////////////////////////////////////////////////////
////NETWORKING

// NETWORK FIRE
bool AZanshinCharacter::ServerFire_Validate(FVector ClientSpawnLocation, FRotator ClientSpawnRotation) { return true; }
void AZanshinCharacter::ServerFire_Implementation(FVector ClientSpawnLocation, FRotator ClientSpawnRotation) { OnFire(ClientSpawnLocation, ClientSpawnRotation); }

void AZanshinCharacter::SpawnBow()
{
	if (Role < ROLE_Authority){
		return;
	}
	
	UWorld* const World = GetWorld();
	if (World != nullptr && Bow == nullptr) {
		AZanshinBow* Spawner = World->SpawnActor<AZanshinBow>(BowClass, FVector::ZeroVector, FRotator::ZeroRotator);
		if (Spawner) {
			Bow = Spawner;
			Bow->OnEquip(this);
		}
	}	
}

bool AZanshinCharacter::ServerPullString_Validate() { return true; }
void AZanshinCharacter::ServerPullString_Implementation() { OnPullString(); }

// For Jet Jump
bool AZanshinCharacter::Server_OnJetJump_Validate() { return true; }
void AZanshinCharacter::Server_OnJetJump_Implementation() 
{ 
	//NetMulticastSetJumpHeight(JetJumpHeight, JetJumpAirControl); 
	FVector JetJumpDirection = GetVelocity().GetSafeNormal() * JetJumpPower;
	JetJumpDirection.Z = JetJumpPower;
	
	LaunchCharacter(JetJumpDirection, true, true);
}

// SERVER JUMP IMPLEMENTATION
void AZanshinCharacter::ServerOnJump_Implementation() { NetMulticastSetJumpHeight(JumpHeight, JumpAirControl); }
bool AZanshinCharacter::ServerOnJump_Validate() { return true; }
bool AZanshinCharacter::NetMulticastSetJumpHeight_Validate(float CurrentJumpHeight, float AirControl) { return true; }

// NETWORK FUNCTION USED TO SET JUMP HEIGHT ON EVERY CLIENT
void AZanshinCharacter::SetJumpHeight(float CurrentJumpHeight, float AirControl)
{
	GetCharacterMovement()->JumpZVelocity = CurrentJumpHeight;
	GetCharacterMovement()->AirControl = AirControl;
}

void AZanshinCharacter::NetMulticastSetJumpHeight_Implementation(float CurrentJumpHeight, float AirControl) { SetJumpHeight(CurrentJumpHeight, AirControl); }

///NETWORK RUN
void AZanshinCharacter::ServerRun_Implementation() { Run(); }
bool AZanshinCharacter::ServerRun_Validate() { return true; }

void AZanshinCharacter::ServerStopRun_Implementation() { StopRun(); }
bool AZanshinCharacter::ServerStopRun_Validate() { return true; }

bool AZanshinCharacter::NetMulticastSetWalkSpeed_Validate(float CurrentWalkSpeed) { return true; }
void AZanshinCharacter::NetMulticastSetWalkSpeed_Implementation(float CurrentWalkSpeed) { SetWalkSpeed(CurrentWalkSpeed); }

void AZanshinCharacter::Client_PlaySound_Implementation(USoundBase* DesiredSound) { 
	if (DesiredSound != nullptr)
		UGameplayStatics::PlaySoundAtLocation(this, DesiredSound, GetActorLocation());
}

bool AZanshinCharacter::Client_PlaySound_Validate(USoundBase* DesiredSound)
{
	if (DesiredSound != nullptr){
		return true;
	} else {
		DesiredSound = ArrowShot;
		return true;
	}
}

void AZanshinCharacter::Client_PlaySound_Attached_Implementation(USoundBase* DesiredSound) 
{ 
	if (DesiredSound != nullptr)
		UGameplayStatics::SpawnSoundAttached(DesiredSound, GetMesh(), FName(TEXT("head")), FVector::ZeroVector, EAttachLocation::KeepRelativeOffset, true, 1.0f, 1.0f, 0.0f, NULL)->bAutoActivate = true;
}

bool AZanshinCharacter::Client_PlaySound_Attached_Validate(USoundBase* DesiredSound)
{
	if (DesiredSound != nullptr){
		return true;
	}
	else {
		DesiredSound = ArrowShot;
		return true;
	}
}

void AZanshinCharacter::SetWalkSpeed(float WalkSpeed) { GetCharacterMovement()->MaxWalkSpeed = WalkSpeed * SpeedAmount; }

bool AZanshinCharacter::NetMulticastHandleRagdollEffect_Validate() { return true; }
void AZanshinCharacter::NetMulticastHandleRagdollEffect_Implementation() { DoRagdoll(); }

void AZanshinCharacter::DoRagdoll()
{
	GetMesh()->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	//GetMesh()->SetAllBodiesSimulatePhysics(true);
	GetMesh()->SetSimulatePhysics(true);
	//GetMesh()->SetSimulatePhysics(true);
	//GetMesh()->WakeAllRigidBodies();
	//GetMesh()->bBlendPhysics = true;
	GetMesh()->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
}

bool AZanshinCharacter::NetMulticastAddForceToRagdoll_Validate(FVector Direction) { return true; }
void AZanshinCharacter::NetMulticastAddForceToRagdoll_Implementation(FVector Direction) { AddForceToRagdoll(Direction); }

// Change Arrow
void AZanshinCharacter::ServerPreviousArrow_Implementation() { PreviousWeapon(); }
bool AZanshinCharacter::ServerPreviousArrow_Validate() { return true;  }

void AZanshinCharacter::ServerNextArrow_Implementation() { NextWeapon(); }
bool AZanshinCharacter::ServerNextArrow_Validate() { return true; }

void AZanshinCharacter::Server_ChangeArrowFromKey_Implementation(int32 ArrowIndexKey) { ChangeArrowFromKey(ArrowIndexKey); }
bool AZanshinCharacter::Server_ChangeArrowFromKey_Validate(int32 ArrowIndexKey) { return true; }

bool AZanshinCharacter::IsFirstPerson() const { return Controller && Controller->IsLocalPlayerController(); }

USkeletalMeshComponent* AZanshinCharacter::GetPawnMesh() const { return IsFirstPerson() ? MeshArms : GetMesh(); }

void AZanshinCharacter::CreateAnimationArrow()
{
	UWorld* const World = GetWorld();
	if (World != nullptr) {
		FActorSpawnParameters Params;
		Params.Owner = GetOwner();
		Params.Instigator = Instigator;

		if (this->IsFirstPerson()){

			if (AnimationArrow != nullptr) AnimationArrow->Destroy();
			if (Arrows.Num() > 0 && Arrows[0] != nullptr){
				AnimationArrow = World->SpawnActor<AZanshinBasicArrow>(Arrows[GetArrowIndex()], FVector(0, 0, 0), FRotator(90, 0, 0), Params);
				AnimationArrow->CreateAnimationArrow(this, this->GetSpecifcPawnMesh(true), ArrowSocket);
				AnimationArrow->SetAnimationCollisionProfile();
			}
		}
	}
}

void AZanshinCharacter::DetachCamera()
{
	if (!this->IsFirstPerson()){
		FirstPersonCameraComponent->DestroyComponent();
		SpringArmComponent->DestroyComponent();
	}
}

void AZanshinCharacter::ServerSetIsReloading_Implementation(bool bNewIsReloading) { SetIsReloading(bNewIsReloading); }
bool AZanshinCharacter::ServerSetIsReloading_Validate(bool bNewIsReloading) { return true; }

void AZanshinCharacter::ServerSetIsChangingArrow_Implementation(bool bNewIsChangingArrow) { SetIsChangingArrow(bNewIsChangingArrow); }
bool AZanshinCharacter::ServerSetIsChangingArrow_Validate(bool bNewIsChangingArrow) { return true; }

int32 AZanshinCharacter::GetArrowIndex()
{
	ArrowIndex = (ArrowIndex + Arrows.Num()) % Arrows.Num();
	return ArrowIndex;
}

void AZanshinCharacter::Server_CancelShot_Implementation() { CancelShot(); }
bool AZanshinCharacter::Server_CancelShot_Validate() { return true; }

void AZanshinCharacter::Server_SetCurrentAmmo_Implementation() { SetCurrentArrowAmmo(); }
bool AZanshinCharacter::Server_SetCurrentAmmo_Validate() { return true; }

void AZanshinCharacter::Server_SetCurrentMaxAmmo_Implementation() { SetCurrentMaxArrowAmmo(); }
bool AZanshinCharacter::Server_SetCurrentMaxAmmo_Validate() { return true; }

// Player Feedback.
void AZanshinCharacter::Client_AddPlayerFeedback_Death_Implementation() { AddPlayerFeedBack_Death(); }
bool AZanshinCharacter::Client_AddPlayerFeedback_Death_Validate() { return true; }

void AZanshinCharacter::Client_AddPlayerFeedback_ReceiveHit_Implementation() { AddPlayerFeedback_ReceiveHit(); }
bool AZanshinCharacter::Client_AddPlayerFeedback_ReceiveHit_Validate() { return true; }

void AZanshinCharacter::Client_AddPlayerFeedback_HitEnemy_Implementation() { AddPlayerFeedback_HitEnemy(); }
bool AZanshinCharacter::Client_AddPlayerFeedback_HitEnemy_Validate() { return true; }

///Feedback Functions.
void AZanshinCharacter::AddPlayerFeedBack_Death()
{
	if (Role < ROLE_Authority){

		return;
	} else 
	{
		//Client_AddPlayerFeedback_Death();
	}
}

void AZanshinCharacter::AddPlayerFeedback_ReceiveHit()
{
	if (Role < ROLE_Authority)
	{
		bSelfHit = true;
		Client_PlaySound_Spawned(CharacterAudioManager->GruntSound[characterIndex], GetActorLocation(), 1.f);
	}
	else
	{
		//Client_AddPlayerFeedback_ReceiveHit();
		bSelfHit = true;
		Client_PlaySound_Spawned(CharacterAudioManager->GruntSound[characterIndex], GetActorLocation(), 1.f);
	}
}

void AZanshinCharacter::AddPlayerFeedback_HitEnemy()
{
	if (Role < ROLE_Authority)
	{
		bEnemyHit = true;
		GetWorld()->GetTimerManager().SetTimer(TimerHandle_ResetEnemyHit, this, &AZanshinCharacter::ResetEnemyHit, TimeBetweenPlayerHit);
	}
	else
	{
		//Client_AddPlayerFeedback_HitEnemy();
		bEnemyHit = true;
		GetWorld()->GetTimerManager().SetTimer(TimerHandle_ResetEnemyHit, this, &AZanshinCharacter::ResetEnemyHit, TimeBetweenPlayerHit);
	}

	AZanshinPlayerState* ZPlayerState = Cast<AZanshinPlayerState>(PlayerState);
	if (ZPlayerState) {
		ZPlayerState->ZanshinPlayerData.ArrowsThatHit++;
	}
}

void AZanshinCharacter::HandleColorChange(){

	AZanshinPlayerController* PC = Cast<AZanshinPlayerController>(GetController());
	AZanshinPlayerState* ZPlayerState = Cast<AZanshinPlayerState>(PlayerState);

	if (PC != nullptr && ZPlayerState != nullptr){

		CharacterTeam = PC->CharacterTeam;

		if (CharacterTeam != 0){
			UpdateMaterial(CharacterTeam);
			UpdateTextRender(ZPlayerState->UserName, CharacterTeam);
			ColorUpdated = true;
			characterIndex = CharacterTeam == 1 ? FMath::RandRange(0, 1) : FMath::RandRange(2, 3);
			}
	}

	else if (CharacterTeam != 0){
		characterIndex = CharacterTeam == 1 ? FMath::RandRange(0, 1) : FMath::RandRange(2, 3);
		UpdateMaterial(CharacterTeam);

		if (ZPlayerState != nullptr)
			UpdateTextRender(ZPlayerState->UserName, CharacterTeam);

		ColorUpdated = true;
	}

	if (ColorUpdated && Role == ROLE_Authority && Bow != nullptr){
		Bow->CurrentColor = CharacterTeam;
	}

}

void AZanshinCharacter::ResetEnemyHit(){ bEnemyHit = false; }

void AZanshinCharacter::ResetSelfHit(){ bSelfHit = false; }

void AZanshinCharacter::Server_MeleeAttack_Implementation() { 
	MeleeAttack();
}
bool AZanshinCharacter::Server_MeleeAttack_Validate() { return true; }

///////////////////////////////////////////////////////////////////////////////
///// Achievements

// Kill Type.
void AZanshinCharacter::Client_UpdateKillType_Implementation(KillType Kill) { BP_Event_UpdateTypeOfKill(Kill); }
bool AZanshinCharacter::Client_UpdateKillType_Validate(KillType Kill) { return true; }

//////////////////////////////////////////////////////////////////////////
// UI Hud Offscreen Detection

void AZanshinCharacter::FindScreenEdgeLocationForWorldLocation(UObject* WorldContextObject, const FVector& InLocation, const float EdgePercent, FVector2D& OutScreenPosition, float& OutRotationAngleDegrees, bool &bIsOnScreen)
{
	bIsOnScreen = false;
	OutRotationAngleDegrees = 0.f;
	FVector2D *ScreenPosition = new FVector2D();

	const FVector2D ViewportSize = FVector2D(GEngine->GameViewport->Viewport->GetSizeXY());
	const FVector2D  ViewportCenter = FVector2D(ViewportSize.X / 2, ViewportSize.Y / 2);

	UWorld* World = GEngine->GetWorldFromContextObject(WorldContextObject);

	if (!World) return;

	APlayerController* PlayerController = (WorldContextObject ? UGameplayStatics::GetPlayerController(WorldContextObject, 0) : NULL);
	ACharacter *PlayerCharacter = static_cast<ACharacter *> (PlayerController->GetPawn());

	if (!PlayerCharacter) return;


	FVector Forward = PlayerCharacter->GetActorForwardVector();
	FVector Offset = (InLocation - PlayerCharacter->GetActorLocation()).GetSafeNormal();

	float DotProduct = FVector::DotProduct(Forward, Offset);
	bool bLocationIsBehindCamera = (DotProduct < 0);

	if (bLocationIsBehindCamera)
	{
		// For behind the camera situation, we cheat a little to put the
		// marker at the bottom of the screen so that it moves smoothly
		// as you turn around. Could stand some refinement, but results
		// are decent enough for most purposes.

		FVector DiffVector = InLocation - PlayerCharacter->GetActorLocation();
		FVector Inverted = DiffVector * -1.f;
		FVector NewInLocation = PlayerCharacter->GetActorLocation() * Inverted;

		NewInLocation.Z -= 5000;

		PlayerController->ProjectWorldLocationToScreen(NewInLocation, *ScreenPosition);
		ScreenPosition->Y = (EdgePercent * ViewportCenter.X) * 2.f;
		ScreenPosition->X = -ViewportCenter.X - ScreenPosition->X;
	}

	PlayerController->ProjectWorldLocationToScreen(InLocation, *ScreenPosition);

	// Check to see if it's on screen. If it is, ProjectWorldLocationToScreen is all we need, return it.
	if (ScreenPosition->X >= 0.f && ScreenPosition->X <= ViewportSize.X
		&& ScreenPosition->Y >= 0.f && ScreenPosition->Y <= ViewportSize.Y)
	{
		OutScreenPosition = *ScreenPosition;
		bIsOnScreen = true;
		return;
	}

	*ScreenPosition -= ViewportCenter;

	float AngleRadians = FMath::Atan2(ScreenPosition->Y, ScreenPosition->X);
	AngleRadians -= FMath::DegreesToRadians(90.f);

	OutRotationAngleDegrees = FMath::RadiansToDegrees(AngleRadians) + 180.f;

	float Cos = cosf(AngleRadians);
	float Sin = -sinf(AngleRadians);

	ScreenPosition = new FVector2D(ViewportCenter.X + (Sin * 150.f), ViewportCenter.Y + Cos * 150.f);

	float m = Cos / Sin;

	FVector2D ScreenBounds = ViewportCenter * EdgePercent;

	if (Cos > 0)
	{
		ScreenPosition = new FVector2D(ScreenBounds.Y / m, ScreenBounds.Y);
	}
	else
	{
		ScreenPosition = new FVector2D(-ScreenBounds.Y / m, -ScreenBounds.Y);
	}

	if (ScreenPosition->X > ScreenBounds.X)
	{
		ScreenPosition = new FVector2D(ScreenBounds.X, ScreenBounds.X*m);
	}
	else if (ScreenPosition->X < -ScreenBounds.X)
	{
		ScreenPosition = new FVector2D(-ScreenBounds.X, -ScreenBounds.X*m);
	}

	*ScreenPosition += ViewportCenter;

	OutScreenPosition = *ScreenPosition;

}

bool AZanshinCharacter::NetMulticast_ImpulseOnBone_Validate(float ImpulseApplied, const FHitResult& Hit, const FVector ArrowDirection) { return true; }
void AZanshinCharacter::NetMulticast_ImpulseOnBone_Implementation(float ImpulseApplied, const FHitResult& Hit, const FVector ArrowDirection) { ImpulseOnBone(ImpulseApplied, Hit, ArrowDirection); }

void AZanshinCharacter::Client_PlaySound_Spawned_Implementation(USoundBase* DesiredSound, FVector Location, float pitchMultiplier)
{
	//We only want one heartbeat.
	if (DesiredSound == CharacterAudioManager->HeartBeat && HeartBeatSound != nullptr)
		return;
	if (DesiredSound == CharacterAudioManager->HeartBeat && HeartBeatSound == nullptr){
		HeartBeatSound = UGameplayStatics::SpawnSoundAttached(DesiredSound, GetMesh(), FName(TEXT("head")), FVector::ZeroVector, EAttachLocation::KeepRelativeOffset, true, 1.0f, pitchMultiplier, 0.0f, NULL);
		HeartBeatSound->bAutoActivate = true;
	}
	else{
		UGameplayStatics::SpawnSoundAttached(DesiredSound, GetMesh(), FName(TEXT("head")), FVector::ZeroVector, EAttachLocation::KeepRelativeOffset, true, 1.0f, pitchMultiplier, 0.0f, NULL)->bAutoActivate = true;
	}
}

bool AZanshinCharacter::Client_PlaySound_Spawned_Validate(USoundBase* DesiredSound, FVector Location, float pitchMultiplier)
{
	if (DesiredSound != nullptr)
	{
		return true;
	}
	else
	{
		return true;
	}
}

bool AZanshinCharacter::Client_UpdateControlPoints_Validate(bool bIsInsideCP, float CurrentPoints, int32 CapturePointLetter){
	return true;
}

void AZanshinCharacter::Client_UpdateControlPoints_Implementation(bool bIsInsideCP, float CurrentPoints, int32 CapturePointLetter){
	if (bIsInsideCP)
		BP_Event_ShowCPFeedback(CurrentPoints, CapturePointLetter);
	else
		BP_Event_HideCPFeedback();
}
