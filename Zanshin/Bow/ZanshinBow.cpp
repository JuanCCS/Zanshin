// Fill out your copyright notice in the Description page of Project Settings.

#include "Zanshin.h"
#include "ZanshinBow.h"
#include "Arrow/ZanshinBasicArrow.h"
#include "Arrow/ZanshinExplosiveArrow.h"
#include "Arrow/ZanshinTeleportArrow.h"

// Sets default values
AZanshinBow::AZanshinBow(const FObjectInitializer& ObjectInitializer) 
	: Super(ObjectInitializer)
{
	bReplicates = true;
	bNetLoadOnClient = true;
	bNetUseOwnerRelevancy = true;

	/*Root = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));
	RootComponent = Root;*/

	Mesh1P = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("WeaponMesh1P"));
	Mesh1P->MeshComponentUpdateFlag = EMeshComponentUpdateFlag::OnlyTickPoseWhenRendered;
	Mesh1P->bChartDistanceFactor = false;
	Mesh1P->bReceivesDecals = false;
	Mesh1P->CastShadow = false;
	Mesh1P->SetCollisionObjectType(ECC_WorldDynamic);
	Mesh1P->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	Mesh1P->SetCollisionResponseToAllChannels(ECR_Ignore);
	RootComponent = Mesh1P;

	Mesh3P = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("WeaponMesh3P"));
	Mesh3P->MeshComponentUpdateFlag = EMeshComponentUpdateFlag::OnlyTickPoseWhenRendered;
	Mesh3P->bChartDistanceFactor = true;
	Mesh3P->bReceivesDecals = false;
	Mesh3P->CastShadow = true;
	Mesh3P->SetCollisionObjectType(ECC_WorldDynamic);
	Mesh3P->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	Mesh3P->SetCollisionResponseToAllChannels(ECR_Ignore);
	//Mesh3P->SetCollisionResponseToChannel(COLLISION_WEAPON, ECR_Block);
	Mesh3P->SetCollisionResponseToChannel(ECC_Visibility, ECR_Block);
	//Mesh3P->SetCollisionResponseToChannel(COLLISION_PROJECTILE, ECR_Block);
	Mesh3P->AttachParent = Mesh1P;

	//bReplicates = true;
	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	bEnemyHit = false;
}

void AZanshinBow::GetLifetimeReplicatedProps(TArray< FLifetimeProperty > & OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(AZanshinBow, ArrowPower);
	DOREPLIFETIME(AZanshinBow, CurrentColor);
}

// Called when the game starts or when spawned
void AZanshinBow::BeginPlay()
{
	Super::BeginPlay();

	EnableFire();

	ArrowIndex = 0;

	MaxArrowInQuiver = Arrows.Num() - 1;

	bCanUpdatePower = false;
	bAimIsShaking = false;
	
	/*for (TSubclassOf<class AZanshinBasicArrow> Arrow : Arrows)
	{
		FArrowData ArrowsDataObject;
		ArrowsDataObject.MaxAmmo = MaxAmmo;
		ArrowsData.Add(Arrow.GetDefaultObject()->GetClass()->GetName(), ArrowsDataObject);
	}*/
}

// Called every frame
void AZanshinBow::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (bCanUpdatePower) {	
		UpdatePower(DeltaTime);
	}

	if (MyPawn == nullptr || MyPawn->GetBow() != this) {
		Destroy();
	}

	/*if (ArrowPowerInSeconds > MaxSecondsAiming) {
		bAimIsShaking = true;
	}*/
}

void AZanshinBow::OnEquip(AZanshinCharacter* Pawn) 
{
	MyPawn = Pawn;
	Instigator = Pawn;
	SetOwner(MyPawn);
		
	AttachMeshToPawn();
}

//////////////////////////////////////////////////////////////////////////
// Enemy Hit Feedback Crosshair

void AZanshinBow::PlayerHit() 
{ 
	bEnemyHit = true; 
	UWorld* const World = GetWorld();
	if (World != NULL)
	{
		World->GetTimerManager().SetTimer(TimerHandle_ResetPlayerHit, this, &AZanshinBow::ResetPlayerHit, TimeBetweenPlayerHit);
	}
}

void AZanshinBow::ResetPlayerHit() { bEnemyHit = false; }

//////////////////////////////////////////////////////////////////////////
// Change arrow

void AZanshinBow::ArrowChanged()
{
	//Only cancel the arrow power if the player is pulling the string
	if (bCanUpdatePower)
	{
		ArrowPower = 0;
		bCanFire = false;
		bCanUpdatePower = false;
	}
}

void AZanshinBow::PreviousWeapon() { ArrowIndex--; }
void AZanshinBow::NextWeapon() { ArrowIndex++; }

void AZanshinBow::AddArrow(TSubclassOf<class AZanshinBasicArrow> Arrow, int32 Ammo) { ArrowsData[Arrow.GetDefaultObject()->GetClass()->GetName()].AddAmmo(Ammo); }

//////////////////////////////////////////////////////////////////////////
// Fire

void AZanshinBow::OnPullString()
{
	bCanUpdatePower = true;
	ArrowPower = 0;
}

void AZanshinBow::EnableFire()
{ 
	bCanFire = true; 
	bTimerStarted = false;
}

void AZanshinBow::DisableFire() { bCanFire = false;}

void AZanshinBow::UpdatePower(float DeltaTime)
{
	/*ArrowPower += DeltaTime * ArrowPowerMultiplier;
	ArrowPower = FMath::Clamp(ArrowPower, 0.0f, ArrowMaxPower);
	ArrowPowerInSeconds += DeltaTime;*/

	ArrowPowerInSeconds += DeltaTime;
	ArrowPower = arrowPowerCurve->GetFloatValue(ArrowPowerInSeconds) * ArrowMaxPower;
}

void AZanshinBow::OnFire(FVector ClientSpawnLocation, FRotator ClientSpawnRotation)
{
	if (MyPawn->Role < ROLE_Authority){
		return;
	}

	//if (bCanFire)
	//{
		bCanFire = false;
		bCanUpdatePower = false;

		SpawnArrow(ClientSpawnLocation, ClientSpawnRotation);
	//}

	/*UWorld* const World = GetWorld();
	if (World != NULL) {
		if (!bTimerStarted){
			bTimerStarted = true;
			World->GetTimerManager().SetTimer(TimerHandle_ShotTimerExpired, this, &AZanshinBow::EnableFire, TimeBetweenShoots);
		}
	}*/
}

void AZanshinBow::SpawnArrow(FVector ClientSpawnLocation, FRotator ClientSpawnRotation)
{
	UWorld* const World = GetWorld();
	if (World != nullptr && 
		MyPawn != nullptr)
	{
		FRotator SpawnRotation = ClientSpawnRotation;
		FVector SpawnLocation = ClientSpawnLocation;

		CallCount++;

		FActorSpawnParameters Params;
		Params.Owner = this->GetOwner();
		Params.Instigator = Instigator;
		
		/*if (bAimIsShaking)
		{
			
			FVector NewLocation = FVector(0.0f, FMath::FRandRange(-ArrowPowerInSeconds, ArrowPowerInSeconds)*5.0f, FMath::FRandRange(-ArrowPowerInSeconds, ArrowPowerInSeconds)*5.0f);

			//Limits the spawn range
			if (NewLocation.Y > 10.0f)
				NewLocation.Y = 10.0f;
			if (NewLocation.Z > 10.0f)
				NewLocation.Z = 10.0f;

			SpawnLocation = NewLocation + SpawnLocation;
			

			FRotator NewRotation = FRotator(FMath::FRandRange(-2.f, 2.f), 0, FMath::FRandRange(-2.f, 2.f));

			SpawnRotation = SpawnRotation + NewRotation;
		}*/

		CurrentArrow = World->SpawnActor<AZanshinBasicArrow>(Arrows[MyPawn->ArrowIndex], SpawnLocation, SpawnRotation, Params);
		CurrentArrow->SetOwningPawnAndBow(MyPawn, this);
		CurrentArrow->SetReplicates(true);

		/*if (Role == ROLE_Authority){
			AnimationArrow->Destroy();
		}*/

		MyPawn->ArrowsData[CurrentArrow->GetClass()->GetName()].AddAmmo(-1);
		ArrowPower = 0;
		ArrowPowerInSeconds = 0.0f;
		bAimIsShaking = false;
	}
}

void AZanshinBow::CreateAnimationArrow()
{
	if (Role < ROLE_Authority){
		return;
	}

	UWorld* const World = GetWorld();
	if (World != nullptr) {
		FActorSpawnParameters Params;
		Params.Owner = GetOwner();
		Params.Instigator = Instigator;

		if (AnimationArrow != nullptr) AnimationArrow->Destroy();

		AnimationArrow = World->SpawnActor<AZanshinBasicArrow>(Arrows[GetArrowIndex()], FVector(0, 0, 0), FRotator(90, 0, 0), Params);
		AnimationArrow->CreateAnimationArrow(MyPawn, MyPawn->GetSpecifcPawnMesh(true), ArrowSocket);
	}	
}

void AZanshinBow::ResetPower()
{
	ArrowPower = 0;
	ArrowPowerInSeconds = 0.0f;
	bAimIsShaking = false;
	bCanUpdatePower = false;
}

void AZanshinBow::AttachMeshToPawn()
{
	//Server_HideMesh();

	if (MyPawn)
	{
		// Remove and hide both first and third person meshes
		DetachMeshFromPawn();

		// For locally controller players we attach both weapons and let the bOnlyOwnerSee, bOwnerNoSee flags deal with visibility.
		FName FirstPersonAttachPoint = MyPawn->GetFirstPersonBowAttachPoint();
		FName ThirdPersonAttachPoint = MyPawn->GetThirdPersonBowAttachPoint();
		USkeletalMeshComponent* PawnMesh1p = MyPawn->GetSpecifcPawnMesh(true);
		USkeletalMeshComponent* PawnMesh3p = MyPawn->GetSpecifcPawnMesh(false);
		Mesh1P->AttachTo(PawnMesh1p, FirstPersonAttachPoint);
		Mesh3P->AttachTo(PawnMesh3p, ThirdPersonAttachPoint);
	}
}

USkeletalMeshComponent* AZanshinBow::GetWeaponMesh() const { return (MyPawn != NULL && MyPawn->IsFirstPerson()) ? Mesh1P : Mesh3P; }

void AZanshinBow::DetachMeshFromPawn()
{
	Mesh1P->DetachFromParent();
	//Mesh1P->SetHiddenInGame(true);

	Mesh3P->DetachFromParent();
	//Mesh3P->SetHiddenInGame(true);
}

int32 AZanshinBow::GetCurrentAmmo()
{
	if (ArrowsData.Contains(Arrows[GetArrowIndex()].GetDefaultObject()->GetClass()->GetName())){
		return ArrowsData[Arrows[GetArrowIndex()].GetDefaultObject()->GetClass()->GetName()].CurrentAmmo;
	}
	else
		return -1;
}

int32 AZanshinBow::GetMaxCurrentAmmo()
{
	if (ArrowsData.Contains(Arrows[GetArrowIndex()].GetDefaultObject()->GetClass()->GetName())){
		return ArrowsData[Arrows[GetArrowIndex()].GetDefaultObject()->GetClass()->GetName()].MaxAmmo;
	}
	else
		return -1;
}

bool AZanshinBow::Client_CreateAnimArrow_Validate()
{
		return true;
}

void AZanshinBow::Client_CreateAnimArrow_Implementation()
{
	UWorld* const World = GetWorld();
	if (World != nullptr) {
		FActorSpawnParameters Params;
		Params.Owner = GetOwner();
		Params.Instigator = Instigator;

		if (AnimationArrow != nullptr) AnimationArrow->Destroy();

		//AnimationArrow = World->SpawnActor<AZanshinBasicArrow>(Arrows[ArrowIndex], FVector(0, 0, 0), FRotator(90, 0, 0), Params);
		//AnimationArrow->CreateAnimationArrow(MyPawn, MyPawn->GetSpecifcPawnMesh(true), ArrowSocket);
	}
}

bool AZanshinBow::NetMulticast_HideAnimArrow_Validate() { return true; }

void AZanshinBow::NetMulticast_HideAnimArrow_Implementation()
{
	if (AnimationArrow != nullptr){
		AnimationArrow->Destroy();
	}
}

int32 AZanshinBow::GetArrowIndex(){

	ArrowIndex = (ArrowIndex + Arrows.Num()) % Arrows.Num();
	return ArrowIndex;
}