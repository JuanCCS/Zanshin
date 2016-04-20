// Fill out your copyright notice in the Description page of Project Settings.

#include "Zanshin.h"
#include "ZanshinDrone.h"


// Sets default values
AZanshinDrone::AZanshinDrone(const class FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	AIControllerClass = AZanshinAIController::StaticClass();

	/*SphereComponent = CreateDefaultSubobject<USphereComponent>(TEXT("SphereComponent"));
	SphereComponent->OnComponentHit.AddDynamic(this, &AZanshinDrone::OnHit);
	SphereComponent->AttachTo(RootComponent);*/
	
 	// Set this pawn to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
}

// Called when the game starts or when spawned
void AZanshinDrone::BeginPlay()
{
	Super::BeginPlay();
}

// Called every frame
void AZanshinDrone::Tick( float DeltaTime )
{
	Super::Tick( DeltaTime );
}

// Called to bind functionality to input
void AZanshinDrone::SetupPlayerInputComponent(class UInputComponent* InputComponent)
{
	Super::SetupPlayerInputComponent(InputComponent);
}

void AZanshinDrone::OnHit(class AActor* OtherActor, class UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit)
{
	Destroy();
}