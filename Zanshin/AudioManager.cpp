// Fill out your copyright notice in the Description page of Project Settings.

#include "Zanshin.h"
#include "Player/ZanshinCharacter.h"
#include "AudioManager.h"


AAudioManager* AAudioManager::Instance;
// Sets default values
AAudioManager::AAudioManager(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	bReplicates = true;
	bNetLoadOnClient = true;
	bNetUseOwnerRelevancy = true;
}

AAudioManager::~AAudioManager(){
	Instance = nullptr;
}

AAudioManager* AAudioManager::Get(UWorld* World){
	if (Instance == nullptr)
	for (TActorIterator<AAudioManager> ActorItr(World); ActorItr; ++ActorItr)
	{
		// Same as with the Object Iterator, access the subclass instance with the * or -> operators.
		Instance = *ActorItr;
	}
	return Instance;
}

// Called when the game starts or when spawned
void AAudioManager::BeginPlay()
{
	Super::BeginPlay();
	
}

// Called every frame
void AAudioManager::Tick( float DeltaTime )
{
	Super::Tick( DeltaTime );
}


// Function to Create Sounds
void AAudioManager::Client_PlaySound_Implementation(USoundBase* DesiredSound, FVector Location,AActor* character)
{ 
	UGameplayStatics::PlaySoundAtLocation(character, DesiredSound, Location);
}

bool AAudioManager::Client_PlaySound_Validate(USoundBase* DesiredSound, FVector Location, AActor* character)
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

void AAudioManager::Client_PlaySound_Spawned_Implementation(USoundBase* DesiredSound, FVector Location, AActor* character,float pitchMultiplier)
{
	AZanshinCharacter* NewCharacter = Cast<AZanshinCharacter>(character);
	if (NewCharacter != nullptr)
		UGameplayStatics::SpawnSoundAttached(DesiredSound, NewCharacter->GetMesh(), FName(TEXT("head")), FVector::ZeroVector, EAttachLocation::KeepRelativeOffset, true, 1.0f, pitchMultiplier, 0.0f, NULL)->bAutoActivate = true;
}

bool AAudioManager::Client_PlaySound_Spawned_Validate(USoundBase* DesiredSound, FVector Location, AActor* character, float pitchMultiplier)
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

void AAudioManager::PlaySound(USoundBase* DesiredSound, FVector Location, AActor* character){
	UGameplayStatics::PlaySoundAtLocation(character, DesiredSound, Location);
}