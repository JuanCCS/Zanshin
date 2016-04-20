// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "GameFramework/Actor.h"
#include "AudioManager.generated.h"


UCLASS()
class ZANSHIN_API AAudioManager : public AActor
{
	GENERATED_BODY()

	// Sets default values for this actor's properties
	AAudioManager(const class FObjectInitializer& ObjectInitializer);

public:
	~AAudioManager();

	TSubclassOf<AAudioManager> BP;

	static AAudioManager* Instance;
	static AAudioManager* Get(UWorld* World);

	template <typename ObjClass>
	static FORCEINLINE ObjClass* LoadObjFromPath(const FName& Path)
	{
		if (Path == NAME_None) return NULL;
		//~
		return Cast<ObjClass>(StaticLoadObject(ObjClass::StaticClass(), NULL, *Path.ToString()));
	}

	// Called when the game starts or when spawned
	virtual void BeginPlay() override;
	
	// Called every frame
	virtual void Tick( float DeltaSeconds ) override;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Arrow Sounds")
	USoundCue* ArrowShot;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Arrow Sounds")
	USoundCue* ArrowSwoosh;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Arrow Sounds")
	USoundCue* ArrowHitEnviro;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Arrow Sounds")
	USoundCue* HeadShotFeedBack;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Arrow Sounds")
	USoundCue* HeadShot;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Arrow Sounds")
	USoundCue* ChestShot;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Misc")
	USoundCue* Tracked;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Bow Sounds")
	USoundCue* PullString;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character Sounds")
	USoundCue* HeartBeat;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character Sounds")
	USoundCue* JetJump;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Melee Sounds")
	USoundCue* MeleeMiss;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Melee Sounds")
	USoundCue* MeleeHit;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Misc")
	USoundCue* ChangingArrowHUD;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Misc")
	USoundCue* Spawn;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Misc")
	USoundCue* HealthPickup;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Misc")
	USoundCue* AmmoPickup;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Capturing Point")
	USoundCue* PointCaptured;

	//////////////////////////////////////////////////////////////////////////
	// Sounds for Announcer

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Announcer")
	USoundCue* BlueCapturedA;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Announcer")
	USoundCue* BlueCapturedB;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Announcer")
	USoundCue* BlueCapturedC;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Announcer")
	USoundCue* RedCapturedA;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Announcer")
	USoundCue* RedCapturedB;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Announcer")
	USoundCue* RedCapturedC;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Announcer")
	USoundCue* RandomAnnouncement;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Announcer")
	USoundCue* OneMinuteRemaining;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Announcer")
	USoundCue* TwoMinutesRemaining;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Announcer")
	USoundCue* KillingSpreeAnnouncer;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Announcer")
	USoundCue* HeadShotAnnouncer;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Announcer")
	USoundCue* MeleeAnnouncer;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Announcer")
	USoundCue* MatchEnd;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Announcer")
	USoundCue* BlueTeamWon;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Announcer")
	USoundCue* RedTeamWon;

	//////////////////////////////////////////////////////////////////////////
	// Array of Sounds for Characters----- 0 for Frankie / 1 for Ryo / 2 for Tom / 3 for Jimmy 

	/** Grunt Sound	**/
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Unique Characters")
	TArray<USoundCue*> GruntSound;

	/** Killing Spree **/
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Unique Characters")
	TArray<USoundCue*> KillingSpree;

	/** Head Shots	**/
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Unique Characters")
	TArray<USoundCue*> HeadShots;

	/** Healing	**/
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Unique Characters")
	TArray<USoundCue*> Healing;

	/** Capturing Point	**/
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Unique Characters")
	TArray<USoundCue*> CapturingPoint;

	/** Healing	**/
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Unique Characters")
	TArray<USoundCue*> Dying;

	void PlaySound(USoundBase* DesiredSound, FVector Location, AActor* character);

	/** Called from the server. Plays a sound at the current location of the client. */
	UFUNCTION(client, reliable, withvalidation)
	void Client_PlaySound(USoundBase* DesiredSound, FVector Location, AActor* character);
	bool Client_PlaySound_Validate(USoundBase* DesiredSound, FVector Location, AActor* character);
	void Client_PlaySound_Implementation(USoundBase* DesiredSound, FVector Location, AActor* character);

	UFUNCTION(client, reliable, withvalidation)
	void Client_PlaySound_Spawned(USoundBase* DesiredSound, FVector Location, AActor* character, float pitchMultiplier);
	bool Client_PlaySound_Spawned_Validate(USoundBase* DesiredSound, FVector Location, AActor* character, float pitchMultiplier);
	void Client_PlaySound_Spawned_Implementation(USoundBase* DesiredSound, FVector Location, AActor* character, float pitchMultiplier);


};
