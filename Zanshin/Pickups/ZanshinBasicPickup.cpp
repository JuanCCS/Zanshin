// Fill out your copyright notice in the Description page of Project Settings.

#include "Zanshin.h"
#include "ZanshinBasicPickup.h"


// Sets default values
AZanshinBasicPickup::AZanshinBasicPickup()
{
	Root = CreateDefaultSubobject<USceneComponent>(TEXT("RootComponent"));

	SphereComponent = CreateDefaultSubobject<USphereComponent>(TEXT("SphereComp"));
	SphereComponent->BodyInstance.SetCollisionProfileName("OverlapOnlyPawn");
	SphereComponent->OnComponentBeginOverlap.AddDynamic(this, &AZanshinBasicPickup::OnOverlapBegin);		// set up a notification for when this component hits something blocking
	SphereComponent->SetNetAddressable();
	SphereComponent->SetIsReplicated(true);
	SphereComponent->AttachTo(Root);

	BaseMesh = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("BaseMesh"));
	BaseMesh->AttachTo(Root);

	TopMesh = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("TopMesh"));
	TopMesh->AttachTo(Root);

	Particles = CreateDefaultSubobject<UParticleSystemComponent>(TEXT("Particles"));
	Particles->AttachTo(Root);

 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
}

// Called when the game starts or when spawned
void AZanshinBasicPickup::BeginPlay()
{
	Super::BeginPlay();

	Activate();
}

// Called every frame
void AZanshinBasicPickup::Tick( float DeltaTime )
{
	Super::Tick( DeltaTime );

	if (!bIsActive) {
		TimeCounter += DeltaTime;
		if (TimeCounter > TimeToRespawn) {
			Activate();
		}
	}
}

void AZanshinBasicPickup::OnOverlapBegin(class AActor* OtherActor, class UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if (bIsActive && Cast<AZanshinCharacter>(OtherActor)) 
	{

		ActiveEffect(OtherActor);

		Deactivate();
	}
}

void AZanshinBasicPickup::Activate()
{
	bIsActive = true;
	TopMesh->SetHiddenInGame(false);
	SetActorHiddenInGame(false);
	Particles->SetActive(true, true);
}

void AZanshinBasicPickup::Deactivate()
{
	bIsActive = false;
	TimeCounter = 0;
	Particles->Deactivate();
	TopMesh->SetHiddenInGame(true);
}

void AZanshinBasicPickup::ActiveEffect(class AActor* OtherActor)
{

}