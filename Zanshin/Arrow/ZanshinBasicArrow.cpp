// Fill out your copyright notice in the Description page of Project Settings.

#include "Zanshin.h"
#include "ZanshinBasicArrow.h"

// Sets default values
AZanshinBasicArrow::AZanshinBasicArrow() : Super()
{
	Root = CreateDefaultSubobject<USceneComponent>(TEXT("RootComponent"));

	// Use a ProjectileMovementComponent to govern this projectile's movement
	ProjectileMovement = CreateDefaultSubobject<UProjectileMovementComponent>(TEXT("ProjectileComponent"));
	ProjectileMovement->UpdatedComponent = Root;
	ProjectileMovement->InitialSpeed = 0.0f;
	ProjectileMovement->MaxSpeed = 6000.f;
	ProjectileMovement->bRotationFollowsVelocity = true;
	ProjectileMovement->bAutoActivate = false;
	ProjectileMovement->bShouldBounce = false;
	ProjectileMovement->bEditableWhenInherited = true;
	ProjectileMovement->SetNetAddressable();
	ProjectileMovement->SetIsReplicated(true);

	// Use a sphere as a simple collision representation
	TipSphereComponent = CreateDefaultSubobject<USphereComponent>(TEXT("SphereComp"));
	TipSphereComponent->InitSphereRadius(2.5f);
	TipSphereComponent->BodyInstance.SetCollisionProfileName("Projectile");
	TipSphereComponent->OnComponentHit.AddDynamic(this, &AZanshinBasicArrow::OnHit);		// set up a notification for when this component hits something blocking
	TipSphereComponent->SetNetAddressable();
	TipSphereComponent->SetIsReplicated(true);

	// Players can't walk on it
	TipSphereComponent->SetWalkableSlopeOverride(FWalkableSlopeOverride(WalkableSlope_Unwalkable, 0.f));
	TipSphereComponent->CanCharacterStepUpOn = ECB_No;

	AnimationArrow = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("AnimationArrow"));
	AnimationArrow->AttachTo(TipSphereComponent);

	FlyingArrow = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("FlyingArrow"));
	FlyingArrow->AttachTo(TipSphereComponent);

	ArrowAudioComponent = CreateDefaultSubobject<UAudioComponent>(TEXT("SoundComp"));
	ArrowAudioComponent->Sound = ArrowSound;
	ArrowAudioComponent->AttachTo(TipSphereComponent);

	// Set as root component
	RootComponent = TipSphereComponent;
	RootComponent->SetNetAddressable();
	RootComponent->SetIsReplicated(true);

	DisableComponentsSimulatePhysics();

	bSpecialArrow = false;

	KillTypeArrow = ArrowKillType::STANDARD;

	PrimaryActorTick.bCanEverTick = true;
}

void AZanshinBasicArrow::BeginPlay() 
{ 
	Super::BeginPlay();

	MaxRadius = TipSphereComponent->GetScaledSphereRadius();
	TipSphereComponent->SetSphereRadius(2.5f);

	SetLifeSpan(LifeSpan);
}

void AZanshinBasicArrow::Tick(float DeltaTime) 
{
	Super::Tick(DeltaTime);
	UpdateCollision();  
}

void AZanshinBasicArrow::SetOwningPawnAndBow(AZanshinCharacter* Pawn, AZanshinBow* Bow)
{
	if (MyPawn != Pawn)
	{
		Instigator = Pawn;
		MyPawn = Pawn;
		MyBow = Bow;

		// net owner for RPC calls
		SetOwner(Pawn);

		TipSphereComponent->IgnoreActorWhenMoving(MyPawn, true);
		TipSphereComponent->IgnoreActorWhenMoving(MyBow, true);

		AnimationArrow->SetVisibility(false);
		AnimationArrow->bOwnerNoSee = true;
		FlyingArrow->SetVisibility(true);
		FlyingArrow->bOnlyOwnerSee = true;		

		SetProjectileComponent(TipSphereComponent, MyBow->ArrowPower);
	}
}

void AZanshinBasicArrow::CreateAnimationArrow(AZanshinCharacter* Pawn, USkeletalMeshComponent* Mesh, FName SocketName)
{
	if (MyPawn != Pawn)
	{
		Instigator = Pawn;
		MyPawn = Pawn;

		// net owner for RPC calls
		SetOwner(Pawn);

		AnimationArrow->SetVisibility(true);
		FlyingArrow->SetVisibility(false);

		ProjectileMovement->DestroyComponent();

		DeactivateTrails();

		SetLifeSpan(0);

		RootComponent->AttachTo(Mesh, SocketName, EAttachLocation::SnapToTarget);
	}
}

void AZanshinBasicArrow::CreateAnimationArrowPickup(USkeletalMeshComponent* Mesh, FName SocketName)
{
	AnimationArrow->SetVisibility(true);
	FlyingArrow->SetVisibility(false);

	ProjectileMovement->DestroyComponent();

	DeactivateTrails();

	SetLifeSpan(0);

	RootComponent->AttachTo(Mesh, SocketName, EAttachLocation::SnapToTarget);
}

float AZanshinBasicArrow::GetDamage() { return Damage; }

///////////////////////////////////////////////////////////////////////
// Movement

void AZanshinBasicArrow::SetProjectileComponent(USphereComponent* TipSphereComponent, float ArrowPower)
{
	if (ArrowPower < 4000.0f) {
		ArrowPower = 4000.0f;
	}

	ProjectileMovement->ProjectileGravityScale = 0.5f;
	ProjectileMovement->UpdatedComponent = TipSphereComponent;
	ProjectileMovement->InitialSpeed = ArrowPower;
	ProjectileMovement->SetVelocityInLocalSpace(FVector(ArrowPower, 0, 0));
	ProjectileMovement->bRotationFollowsVelocity = true;
	ProjectileMovement->Activate();
	ActiveShotSound();

	StartPosition = GetActorLocation();
}

void AZanshinBasicArrow::UpdateCollision()
{
	FVector OutDir;
	float Distance;
	(StartPosition - GetActorLocation()).ToDirectionAndLength(OutDir, Distance);
	float DistancePercentage = Distance * 100 / MaxDistanceCollision;
	float Radius = MaxRadius * DistancePercentage / 100;
	TipSphereComponent->SetSphereRadius(FMath::Clamp<float>(Radius, 2.5, MaxRadius));
}

void AZanshinBasicArrow::SetAnimationCollisionProfile(){
	TipSphereComponent->SetCollisionProfileName("AnimationArrow");
}

void AZanshinBasicArrow::OnHit(AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit)
{
	if (Role < ROLE_Authority){
		return;
	}

	if (OtherActor != nullptr) 
	{
		AZanshinCharacter* Enemy = Cast<AZanshinCharacter>(OtherActor);
		if (Enemy != nullptr && MyPawn != nullptr && MyPawn->GetTeam() != Enemy->GetTeam())
		{
			NetMulticast_SpecialPower(Enemy, Hit);
			//Add all feedback to the player here.
			if (!Enemy->bIsDead)
			{
				MyPawn->AddPlayerFeedback_HitEnemy();
				Enemy->AddPlayerFeedback_ReceiveHit();

				StuckOnPlayer(OtherComp);
			}
		}
		else if (OtherActor != this)
		{
			NetMulticast_SpecialPowerHitEnvironment(Hit);
		}
	}

	SetHitPosition(Hit.ImpactPoint);

	SetLifeSpan(LifeSpan);
	
	ArrowAudioComponent->Activate(false);
	ArrowAudioComponent->bAutoActivate = false;
	ArrowAudioComponent->Stop();	
}

void AZanshinBasicArrow::SetHitPosition(FVector ImpactPoint)
{
	FVector Impact = ImpactPoint + (GetActorForwardVector() * 40);
	DeactivateTrails();
	SetActorLocation(Impact);
}

void AZanshinBasicArrow::ActiveShotSound()
{
	// try and play the sound if specified
	if (ArrowSound != NULL) 
	{
		ArrowAudioComponent->bAutoActivate = true;
		ArrowAudioComponent->Activate(true);

		ArrowAudioComponent->Play(0.0f);
	}
}

///////////////////////////////////////////////////////////////////////
// Special Power Active

void AZanshinBasicArrow::ActivateSpecialPower() { bSpecialArrow = true; }

///////////////////////////////////////////////////////////////////////
// Effects

void AZanshinBasicArrow::StuckOnPlayer(UPrimitiveComponent* OtherComp)
{
	if (OtherComp != NULL) {
		RootComponent->AttachTo(OtherComp, FName(*OtherComp->GetName()), EAttachLocation::KeepWorldPosition);
	}

	//Timer for the Deactivation
	UWorld* World = GetWorld();
	if (World) {
		World->GetTimerManager().SetTimer(DeactivateTimerHandle, this, &AZanshinBasicArrow::NetMulticast_DeactivateTrails, DeactivationDelay);
	}
}

//Deactivate the Particle System Components
void AZanshinBasicArrow::DeactivateTrails()
{
	TArray<USceneComponent*> Childrens;
	GetRootComponent()->GetChildrenComponents(true, Childrens);
	if (Childrens.Num() != 0)
	{
		for (USceneComponent* Actor : Childrens)
		{
			UParticleSystemComponent* Particles = Cast<UParticleSystemComponent>(Actor);
			if (Particles != NULL)
			{
				Particles->Deactivate();
			}
		}
	}
}

void AZanshinBasicArrow::SpecialPower(AZanshinCharacter* Enemy, const FHitResult& Hit) { }
void AZanshinBasicArrow::SpecialPowerHitEnvironment(const FHitResult& Hit) 
{

}
void AZanshinBasicArrow::ActivatePassivePower(AZanshinCharacter* Enemy) { }

///////////////////////////////////////////////////////////////////////
// NETWORKING

void AZanshinBasicArrow::Server_SetLifeSpan_Implementation(float DesiredLifeSpan) { SetLifeSpan(DesiredLifeSpan); }
bool AZanshinBasicArrow::Server_SetLifeSpan_Validate(float DesiredLifeSpan) { return true; }

void AZanshinBasicArrow::NetMulticast_SpecialPower_Implementation(AZanshinCharacter* Enemy, const FHitResult& Hit) { SpecialPower(Enemy, Hit); }
bool AZanshinBasicArrow::NetMulticast_SpecialPower_Validate(AZanshinCharacter* Enemy, const FHitResult& Hit) { return true; }

void AZanshinBasicArrow::NetMulticast_SpecialPowerHitEnvironment_Implementation(const FHitResult& Hit) { 
	if (MyPawn != nullptr){
		MyPawn->Client_PlaySound(MyPawn->CharacterAudioManager->ArrowHitEnviro);
	}
	SpecialPowerHitEnvironment(Hit); }
bool AZanshinBasicArrow::NetMulticast_SpecialPowerHitEnvironment_Validate(const FHitResult& Hit) { return true; }

void AZanshinBasicArrow::NetMulticast_DeactivateTrails_Implementation() { DeactivateTrails(); }
bool AZanshinBasicArrow::NetMulticast_DeactivateTrails_Validate() { return true; }