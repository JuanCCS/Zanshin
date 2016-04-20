// Fill out your copyright notice in the Description page of Project Settings.

#include "Zanshin.h"
#include "ZanshinAIController.h"

// Sets default values
AZanshinAIController::AZanshinAIController(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	BlackboardComponent = CreateDefaultSubobject<UBlackboardComponent>(TEXT("BlackboardComponent"));

	BehaviorTreeComponent = CreateDefaultSubobject<UBehaviorTreeComponent>(TEXT("BehaviorTreeComponent"));

	PathIndex = -1;

	bWantsPlayerState = true;
}

//////////////////////////////////////////////////////////////////////////
// Basic

void AZanshinAIController::PostInitializeComponents()
{
	Super::PostInitializeComponents();
}

// Called when the game starts or when spawned
void AZanshinAIController::BeginPlay()
{
	Super::BeginPlay();

	for (TActorIterator<ATargetPoint> TargetIt(GetWorld()); TargetIt; ++TargetIt) {
		LocationsToWalk.Add(*TargetIt);
	}

	LocationsToWalk.Sort([this](const ATargetPoint& Item1, const ATargetPoint& Item2) {
		return FMath::FRand() < 0.5f;
	});
}

void AZanshinAIController::BeginInactiveState()
{
	Super::BeginInactiveState();

	StopMovement();

	AGameState* GameState = GetWorld()->GameState;
	const float MinRespawnDelay = (GameState && GameState->GameModeClass) ? GetDefault<AGameMode>(GameState->GameModeClass)->MinRespawnDelay : 1.0f;
	GetWorldTimerManager().SetTimer(TimerHandle_Respawn, this, &AZanshinAIController::Respawn, MinRespawnDelay);
}

void AZanshinAIController::Respawn()
{
	GetWorld()->GetAuthGameMode()->RestartPlayer(this);
}

void AZanshinAIController::Possess(class APawn* InPawn)
{
	Super::Possess(InPawn);
	AZanshinBot* Character = Cast<AZanshinBot>(InPawn);

	if (Character && Character->BehaviorTree) {
		BlackboardComponent->InitializeBlackboard(*Character->BehaviorTree->BlackboardAsset);
		BehaviorTreeComponent->StartTree(*(Character->BehaviorTree));
		RunBehaviorTree(Character->BehaviorTree);

		LocationID = BlackboardComponent->GetKeyID("Destination");
	}

	AZanshinDrone* Drone = Cast<AZanshinDrone>(InPawn);

	if (Drone && Drone->BehaviorTree) {
		BlackboardComponent->InitializeBlackboard(*Drone->BehaviorTree->BlackboardAsset);
		BehaviorTreeComponent->StartTree(*(Drone->BehaviorTree));
		RunBehaviorTree(Drone->BehaviorTree);

		LocationID = BlackboardComponent->GetKeyID("Destination");
	}
}

void AZanshinAIController::FindNextPathNode()
{
	PathIndex++;
	if (!LocationsToWalk.IsValidIndex(PathIndex)) {
		PathIndex = 0;
	}

	SetLocation();
}

void AZanshinAIController::SetLocation()
{
	ATargetPoint* NewTarget = LocationsToWalk[PathIndex];
	if (NewTarget)
	{
		FVector NewLocation = NewTarget->GetActorLocation();
		BlackboardComponent->SetValue<UBlackboardKeyType_Vector>(LocationID, NewLocation);
	}
}

void AZanshinAIController::OnPullString()
{
	AZanshinBot* character = Cast<AZanshinBot>(GetPawn());
	if (character) {
		character->OnPullString();

		UWorld* World = GetWorld();
		if (World) {
			PullStringTime = 0.15f;
			World->GetTimerManager().SetTimer(PullStringTimerHandle, this, &AZanshinAIController::OnFire, PullStringTime);
		}
	}
}

void AZanshinAIController::OnFire()
{
	AZanshinBot* character = Cast<AZanshinBot>(GetPawn());
	if (character) {
		character->OnFire(character->GetActorLocation(), character->GetActorRotation());
	}
}