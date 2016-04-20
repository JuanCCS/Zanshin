// Fill out your copyright notice in the Description page of Project Settings.

#include "Zanshin.h"
#include "ZanshinExplosiveArrow.h"

AZanshinExplosiveArrow::AZanshinExplosiveArrow() : Super()
{
	ExplosiveParticle = CreateDefaultSubobject<UParticleSystemComponent>(TEXT("Explosive Particle"));
	ExplosiveParticle->AttachTo(TipSphereComponent);

	//Sphere collider for the explosion
	SphereCollider = CreateDefaultSubobject<USphereComponent>(TEXT("Sphere Collider"));
	SphereCollider->AttachTo(TipSphereComponent);
	SphereCollider->SetSphereRadius(250.0f);

	KillTypeArrow = ArrowKillType::EXPLOSIVE;
}

void AZanshinExplosiveArrow::BeginPlay()
{
	Super::BeginPlay();	
	SphereCollider->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	SphereCollider->OnComponentBeginOverlap.AddDynamic(this, &AZanshinExplosiveArrow::OnOverlapBegin);

	bIsTimerSet = false;
	bEndColision = false;
}

///////////////////////////////////////////////////////////////////////
// Collision Detection

void AZanshinExplosiveArrow::OnOverlapBegin(class AActor* OtherActor, class UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	AZanshinCharacter* Enemy = Cast<AZanshinCharacter>(OtherActor);
	if (Enemy != nullptr && !Enemy->IsPendingKill()) {
		Enemies.AddUnique(Enemy);
	}
}

///////////////////////////////////////////////////////////////////////
// Effects

void AZanshinExplosiveArrow::SpecialPower(AZanshinCharacter* Enemy, const FHitResult& Hit) { 
	CreateExplosionRadius(Hit); }

void AZanshinExplosiveArrow::SpecialPowerHitEnvironment(const FHitResult& Hit) { 
	CreateExplosionRadius(Hit); }

void AZanshinExplosiveArrow::ActivatePassivePower(AZanshinCharacter* Enemy) { }

void AZanshinExplosiveArrow::CreateExplosionRadius(const FHitResult& Hit)
{	
	BP_Event_TriggerExplosion();
	SphereCollider->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	UGameplayStatics::PlaySoundAtLocation(this, ExplosionSound, SphereCollider->GetComponentLocation(), 1.0f, 1.0f, 0.0f, ExplosionSoundAtt);
	UGameplayStatics::PlayWorldCameraShake(this, ExplosionShake, Hit.ImpactPoint, 0.0f, ExplosionRadiusShake, 1.0, true);
	if (MyPawn != nullptr){
		UWorld* const World = MyPawn->GetWorld();
		if (World != NULL) {
			World->GetTimerManager().SetTimer(TimerHandle_EndColision, this, &AZanshinExplosiveArrow::EndColision, 0.05f);
		}
	}
}

///////////////////////////////////////////////////////////////////////
// Area of Effect Damage
void AZanshinExplosiveArrow::DoAOEDamage()
{
	if (Role < ROLE_Authority) {
		return;
	}

	for (auto& Enemy : Enemies) 
	{
		if (Enemy != nullptr) {
			Enemy->SetParticle(ParticleEffectOnCharacter);
			Enemy->DealDamage(DamageExplosion, this);
		}
	}
	
	Enemies.Empty();
}

void AZanshinExplosiveArrow::EndColision() 
{
	bEndColision = true;
	SphereCollider->SetCollisionEnabled(ECollisionEnabled::NoCollision); 

	DoAOEDamage();
}