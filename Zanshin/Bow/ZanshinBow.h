// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "Player/ZanshinCharacter.h"
#include "GameFramework/Actor.h"
#include "Networking/NetworkDebugInterface.h"
#include "ZanshinBow.generated.h"

class AZanshinCharacter;

UCLASS()
class ZANSHIN_API AZanshinBow : public AActor, public INetworkDebugInterface
{
	GENERATED_BODY()

	/** weapon mesh: 1st person view */
	UPROPERTY(VisibleDefaultsOnly, Category = Mesh)
	USceneComponent* Root;

public:
	//////////////////////////////////////////////////////////////////////////
	// Constructor

	// Sets default values for this actor's properties
	AZanshinBow(const FObjectInitializer& ObjectInitializer);

	UPROPERTY(EditDefaultsOnly,BlueprintReadWrite, Category = UI)
	bool bEnemyHit;

	UPROPERTY(BlueprintReadWrite, replicated, Category = UI)
	int32 CurrentColor;

	UFUNCTION(BlueprintImplementableEvent, meta = (displayname = "KillCam_Visibility"))
	void BP_Event_ShowSelfOnKillCam();

	//////////////////////////////////////////////////////////////////////////
	// Arrow
	
	/** The arrow to spawn */
	UPROPERTY(EditDefaultsOnly, Category = Config)
	TArray<TSubclassOf<class AZanshinBasicArrow>> Arrows;

	/** Max ammo */
	UPROPERTY(EditDefaultsOnly,BlueprintReadWrite, Category = Config)
	int32 MaxAmmo;

	/** How much power the arrow will have.*/
	UPROPERTY(EditAnywhere, Replicated, BlueprintReadWrite, Category = Gameplay)
	float ArrowPower;

	/** Time between shoots. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Gameplay)
	float TimeBetweenShoots;

	/** The multiplier to use with deltatime to set the new power of the arrow. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Gameplay)
	float ArrowPowerMultiplier;

	/** The max power of the arrow. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Gameplay)
	float ArrowMaxPower;

	bool bTimerStarted = false;

	UPROPERTY(EditAnywhere, Category = Gameplay)
		FName ArrowSocket;

	/** [Server] Get Pawn Info. */
	void OnEquip(AZanshinCharacter* Pawn);

	UFUNCTION(BlueprintCallable, Category = "Arrow")
	void CreateAnimationArrow();

	//////////////////////////////////////////////////////////////////////////
	// Enemy Hit Feedback Crosshair

	UFUNCTION(BlueprintCallable, Category = Gameplay)
	void PlayerHit();

	UFUNCTION(BlueprintCallable, Category = Gameplay)
	void ResetPlayerHit();

	/** Handle for efficient management of ShotTimerExpired timer */
	FTimerHandle TimerHandle_ResetPlayerHit;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Gameplay)
	float TimeBetweenPlayerHit;

	//////////////////////////////////////////////////////////////////////////
	// Change arrow

	void ArrowChanged();

	void PreviousWeapon();
	void NextWeapon();

	void AddArrow(TSubclassOf<class AZanshinBasicArrow> Arrow, int32 Ammo);
	
	UFUNCTION(BlueprintImplementableEvent, meta = (DisplayName = "UpdateString"))
	void BP_Event_UpdateString(float arrowIndex);



	//////////////////////////////////////////////////////////////////////////
	// Aim Penalty

	//boolean to check if the player is holding the fire for too long
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Aim)
	bool bAimIsShaking;

	//float to check the time in seconds the player is holding the power
	float ArrowPowerInSeconds; 

	//float to check the maximum time without penalty while aiming
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Aim)
	float MaxSecondsAiming;

	//////////////////////////////////////////////////////////////////////////
	// Fire 

	void OnPullString();
	
	/** [Server] Spawn arrow. */
	void OnFire(FVector ClientSpawnLocation, FRotator ClientSpawnRotation);
	
	/** Enable the player to shot */
	void EnableFire();

	void DisableFire();

	void ResetPower();

	// Called when the game starts or when spawned
	virtual void BeginPlay() override;
	virtual void Tick(float DeltaSeconds) override;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Particle)
	UCurveFloat* arrowPowerCurve;

	//////////////////////////////////////////////////////////////////////////
	// Getter and Setter

	FORCEINLINE float GetArrowPower() const { return ArrowPower; }

	UFUNCTION(BlueprintCallable, Category = Ammo)
	int32 GetCurrentAmmo();

	UFUNCTION(BlueprintCallable, Category = Ammo)
	int32 GetMaxCurrentAmmo();

	FORCEINLINE void SetArrowIndex(int32 ArrowIndexValue) { ArrowIndex = ArrowIndexValue; };

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Power)
	bool bCanUpdatePower;

private:

	/** pawn owner */
	UPROPERTY(Transient)
	class AZanshinCharacter* MyPawn;

	/** weapon mesh: 1st person view */
	UPROPERTY(VisibleDefaultsOnly, Category = Mesh)
	USkeletalMeshComponent* Mesh1P;

	/** weapon mesh: 3rd person view */
	UPROPERTY(VisibleDefaultsOnly, Category = Mesh)
	USkeletalMeshComponent* Mesh3P;

	//////////////////////////////////////////////////////////////////////////
	// Gameplay

	int32 ArrowIndex;
	int32 MaxArrowInQuiver;
	class AZanshinBasicArrow* CurrentArrow;
	class AZanshinBasicArrow* AnimationArrow;	

	bool bCanFire;

	TMap<FString, FArrowData> ArrowsData;

	void UpdatePower(float DeltaTime);
	void SpawnArrow(FVector ClientSpawnLocation, FRotator ClientSpawnRotation);

	/** Handle for efficient management of ShotTimerExpired timer */
	FTimerHandle TimerHandle_ShotTimerExpired;

	/** Attach the bow to the player and set the visibility */
	void AttachMeshToPawn();

	/** Detach the mesh so it can be attach again and set the visibility */
	void DetachMeshFromPawn();
	                                                          
	//////////////////////////////////////////////////////////////////////////
	/// NETOWRK

	int32 CallCount = 0;

public:

	public:

	/** Create the animation arrow on the client. */
	UFUNCTION(client, withvalidation, reliable)
	void Client_CreateAnimArrow();
	bool Client_CreateAnimArrow_Validate();
	void Client_CreateAnimArrow_Implementation();

	/** Hide anim arrow. */
	UFUNCTION(netmulticast, withvalidation, reliable)
		void NetMulticast_HideAnimArrow();
	bool NetMulticast_CreateAnimArrow_Validate();
	void NetMulticast_CreateAnimArrow_Implementation();

	USkeletalMeshComponent* GetWeaponMesh() const;

	/** Keep index within bounds of array */
	int32 GetArrowIndex();

};