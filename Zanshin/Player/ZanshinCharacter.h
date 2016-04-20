// Copyright 1998-2015 Epic Games, Inc. All Rights Reserved.
#pragma once

#include "Zanshin.h"
#include "GameFramework/Character.h"
#include "Networking/NetworkDebugInterface.h"
#include "AudioManager.h"
#include "GameMode/ZanshinGameInstance.h"
#include "GameMode/ZanshinGameMode.h"
#include "Networking/ZanshinNetRepCharacter.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "ZanshinCharacter.generated.h"

class UInputComponent;
class AZanshinBasicArrow;

USTRUCT()
struct FArrowData
{
	GENERATED_USTRUCT_BODY()

	/** The arrow to spawn */
	UPROPERTY(EditDefaultsOnly, Category = Config)
	AZanshinBasicArrow* Arrow;

	/** Current ammo */
	UPROPERTY(EditDefaultsOnly, Category = Config)
	int32 CurrentAmmo;

	/** Max ammo */
	int32 MaxAmmo;

	int32 AddAmmo(int32 Ammo)
	{

		int32 actualAmmo = MaxAmmo - CurrentAmmo;

		CurrentAmmo += Ammo;
		CurrentAmmo = FMath::Clamp<int32>(CurrentAmmo, 0, MaxAmmo);

		return actualAmmo > Ammo? Ammo: actualAmmo;

	}

	/** Defaults. */
	FArrowData()
	{
		CurrentAmmo = 0;
		MaxAmmo = 30;
	}
};

UCLASS(config = Game)
class AZanshinCharacter : public AZanshinNetRepCharacter
{
	GENERATED_BODY()

	/** The arrow to spawn */
	UPROPERTY(EditDefaultsOnly, Category = Config)
	TArray<TSubclassOf<class AZanshinBasicArrow>> Arrows;
	
	//////////////////////////////////////////////////////////////////////////
	// Components

	///** Pawn mesh: 1st person view (arms; seen only by self) */
	UPROPERTY(VisibleDefaultsOnly, Category = Mesh)
	class USkeletalMeshComponent* MeshArms;
	
	/** First person camera */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
	class UCameraComponent* FirstPersonCameraComponent;		

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
	class UCameraComponent* KillerCameraComponent;

public:

	/** Know if a player is dead or alive */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly,replicated)
	bool bIsDead = false;

	UPROPERTY(VisibleAnywhere, replicated, BlueprintReadOnly, Category = Arrow, meta = (AllowPrivateAccess = "true"))
	int32 currentArrow;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Arrow, meta = (AllowPrivateAccess = "true"))
	int32 maxArrowNavigation;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
	float currentCameraFOV=90.0f;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
	class UArrowComponent* ArrowSocketComponent;

	//////////////////////////////////////////////////////////////////////////
	// Constructor

	AZanshinCharacter(const class FObjectInitializer& ObjectInitializer);

	/** Melee Range Box */
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = Melee)
	class UBoxComponent* MeleeRangeCollider;

	//////////////////////////////////////////////////////////////////////////
	// Arrow Detection Sphere Sound
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Audio)
	class USphereComponent* ArrowDetection;

	//////////////////////////////////////////////////////////////////////////
	// Ammo

	UPROPERTY(EditAnywhere, replicated, BlueprintReadWrite, Category = Arrow)
	int32 currentArrowAmmo;

	UPROPERTY(EditAnywhere, replicated, BlueprintReadWrite, Category = Arrow)
	int32 maxCurrentArrowAmmo;

	UFUNCTION(BlueprintCallable, Category = Arrow)
	void SetCurrentArrowAmmo();

	UFUNCTION(BlueprintCallable, Category = Arrow)
	void SetCurrentMaxArrowAmmo();

	TMap<FString, FArrowData> ArrowsData;

	void AddArrow(TSubclassOf<class AZanshinBasicArrow> Arrow, int32 Ammo);

	UFUNCTION(client, unreliable, withvalidation)
	void Client_AddAmmo(TSubclassOf<class AZanshinBasicArrow> Arrow, int32 AmmoAmount);
	bool Client_AddAmmo_Validate(TSubclassOf<class AZanshinBasicArrow> Arrow, int32 AmmoAmount);
	void Client_AddAmmo_Implementation(TSubclassOf<class AZanshinBasicArrow> Arrow, int32 AmmoAmount);
	
	UFUNCTION(BlueprintCallable, Category = ArrowAmmo)
	int32 SendArrowsAmmo(int32 ArrowIndex);
	
	/** Max ammo */
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = Config)
	int32 MaxAmmo;
	
	//////////////////////////////////////////////////////////////////////////
	// Melee

	UPROPERTY(EditAnywhere, BlueprintReadWrite, replicated, Category = Achievement)
	bool bMeleeHitAttacker;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, replicated, Category = Achievement)
	bool bMeleeHitDefender;

	//////////////////////////////////////////////////////////////////////////
	// Aim

	/** Base turn rate, in deg/sec. Other scaling may affect final turn rate. */
	UPROPERTY(EditAnywhere, replicated, BlueprintReadWrite, Category = Camera)
	float BaseTurnRate;

	/** Base look up/down rate, in deg/sec. Other scaling may affect final rate. */
	UPROPERTY(EditAnywhere, replicated, BlueprintReadWrite, Category = Camera)
	float BaseLookUpRate;

	/** get aim offsets */
	UFUNCTION(BlueprintCallable, Category = "Game|Weapon")
	FRotator GetAimOffsets() const;

	/** Spring Arm Component*/
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = SpringArm, meta = (AllowPrivateAccess = "true"))
	class USpringArmComponent* SpringArmComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = SpringArm, meta = (AllowPrivateAccess = "true"))
	class USpringArmComponent* KillerSpringArmComponent;
	
	//////////////////////////////////////////////////////////////////////////
	// UI Hud Offscreen Detection

	/**
	* Converts a world location to screen position for HUD drawing. This differs from the results of FSceneView::WorldToScreen in that it returns a position along the edge of the screen for offscreen locations
	*
	* @param		InLocation	- The world space location to be converted to screen space
	* @param		EdgePercent - How close to the edge of the screen, 1.0 = at edge, 0.0 = at center of screen. .9 or .95 is usually desirable
	* @outparam	OutScreenPosition - the screen coordinates for HUD drawing
	* @outparam	OutRotationAngleDegrees - The angle to rotate a hud element if you want it pointing toward the offscreen indicator, 0° if onscreen
	* @outparam	bIsOnScreen - True if the specified location is in the camera view (may be obstructed)
	*/
	UFUNCTION(BlueprintPure, meta = (WorldContext = "WorldContextObject", CallableWithoutWorldContext), Category = "HUD|Util")
	static void FindScreenEdgeLocationForWorldLocation(UObject* WorldContextObject, const FVector& InLocation, const float EdgePercent, FVector2D& OutScreenPosition, float& OutRotationAngleDegrees, bool &bIsOnScreen);

	//////////////////////////////////////////////////////////////////////////
	// Gameplay

	UPROPERTY(EditAnywhere, Category = Gameplay)
	float LifeSpan;

	/** Current health of the Pawn. */
	UPROPERTY(EditAnywhere, replicated, BlueprintReadWrite, Category = Gameplay)
	float Health;

	UPROPERTY(EditAnywhere, replicated, BlueprintReadWrite, Category = Gameplay)
	float SelfHeal;

	/** Max health of the Pawn. */
	UPROPERTY(EditAnywhere, replicated, BlueprintReadWrite, Category = Gameplay)
	float MaxHealth;

	/** Current health of the Pawn. */
	UPROPERTY(EditAnywhere, replicated, BlueprintReadWrite, Category = Damage)
	float ActualDamageText;

	/** The team for each character **/
	UPROPERTY(EditAnywhere, replicated, BlueprintReadWrite, Category = Gameplay)
	int32 CharacterTeam;

	UPROPERTY(EditAnywhere, Category = Gameplay)
	int32 ForceToRagdoll;

	UPROPERTY(EditAnywhere, Category = Gameplay)
	float FastShotTimer;

	UPROPERTY(EditAnywhere, Category = Gameplay)
	float FastShotLimit;

	/** Time for the next jet jump */
	UPROPERTY(EditAnywhere, Category = Gameplay)
	float TimeToJetJump;

	/** Control the time of the jet jump, store the current time of the jet jump */
	UPROPERTY(EditAnywhere, Category = Gameplay)
	float JetJumpTime;

	UPROPERTY(EditAnywhere, Category = Gameplay)
	float JetJumpPower;

	/** Say if the player can jet jump or not */
	UPROPERTY(EditAnywhere, Category = Gameplay)
	bool bCanJetJump;

	bool bAllowJetJump;

	UPROPERTY(EditAnywhere, Category = Gameplay)
	float TimeToAllowJetJump;

	void AddHealth(int16 Amount);

	void ResetArrowSocket();

	void ChangeCameraActive(bool bIsFirstPerson);

	//////////////////////////////////////////////////////////////////////////
	// Capturing

	UFUNCTION(BlueprintImplementableEvent, meta = (DisplayName = "StartCapturingControlPoint"))
	void BP_Event_StartCapturingControlPoint();

	UFUNCTION(BlueprintImplementableEvent, meta = (DisplayName = "EndCapturingControlPoint"))
	void BP_Event_EndCapturingControlPoint();

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Gameplay)
	bool bIsCapturing;

	//////////////////////////////////////////////////////////////////////////
	// Ammo Picked Up
	UFUNCTION(BlueprintImplementableEvent, meta = (DisplayName = "AddAmmo_HUD"))
	void BP_Event_AddAmmoToHud(TSubclassOf<class AZanshinBasicArrow> Arrow, int32 Ammo);

	//////////////////////////////////////////////////////////////////////////
	// Melee Float Text Damage
	UFUNCTION(BlueprintImplementableEvent, meta = (DisplayName = "ShowFloatDamageForMelee"))
	void BP_Event_ShowFloatDamageForMelee(AZanshinCharacter* Actor, float Damage);


	//////////////////////////////////////////////////////////////////////////
	// Character Sound Index

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = CharacterSound)
	int32 characterIndex;

	//////////////////////////////////////////////////////////////////////////
	// Sounds

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Audio)
	AAudioManager* CharacterAudioManager;

	UFUNCTION(BlueprintCallable, Category = Audio)
	void CheckHealth();

	/** Sound to play each time we hit a headshot. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Gameplay)
	class USoundBase* HeadshotSound;

	/** Sound to play each time we hit the chest. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Gameplay)
	class USoundBase* ChestSound;

	/** Sound to play each time we hit the chest. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Gameplay)
	class USoundBase* JetJumpSound;

	/** Sound to play each time we hit a headshot. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Gameplay)
	class USoundBase* PullStringSound;

	/** Sound to play each time we spawn an arrow. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Gameplay)
	class USoundBase* ArrowShot;
	
	/** Sound to play each time we spawn an arrow. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Gameplay)
	class USoundBase* MeleeHitSound;

	/** Sound to play each time we spawn an arrow. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Gameplay)
	class USoundBase* MeleeMissSound;

	/** Sound to play to check health / heart beat */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Gameplay)
	class USoundCue* HeartBeat;

	/** Sound to play to check health / heart beat */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Gameplay)
	class USoundCue* GruntSound;

	///////////////////////////////////////////////////////////////////////
	// Achievement Detection
	UFUNCTION(BlueprintImplementableEvent, meta = (DisplayName = "Update_TypeOfKill"))
	void BP_Event_UpdateTypeOfKill(KillType CurrentKillType);

	///////////////////////////////////////////////////////////////////////
	// Melee Detection
	UFUNCTION()
	void OnOverlapBegin(class AActor* OtherActor, class UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

	///////////////////////////////////////////////////////////////////////
	// Melee Detection
	UFUNCTION()
	void OnOverlapBeginAudio(class AActor* OtherActor, class UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);


	//////////////////////////////////////////////////////////////////////////
	// Bow

	/** Projectile class to spawn. */
	UPROPERTY(EditDefaultsOnly, Category = Gameplay)
	TSubclassOf<class AZanshinBow> BowClass;

	void SpawnBow();

	//////////////////////////////////////////////////////////////////////////
	// Change Arrow 

	UFUNCTION(BlueprintCallable, Category = Arrow)
	void PreviousWeapon();

	UFUNCTION(BlueprintCallable, Category = Arrow)
	void NextWeapon();

	//////////////////////////////////////////////////////////////////////////
	// Fire

	void OnPullString();

	/** Fires a projectile. */
	void OnFire(FVector ClientSpawnLocation, FRotator ClientSpawnRotation);

	/** Cancel the Strin Pull */
	void CancelShot();

	void AfterFire();

	void AfterFireCancel();

	UPROPERTY(EditAnywhere, replicated, BlueprintReadWrite, Category = Arrow)
	bool bChangingArrow;

	//////////////////////////////////////////////////////////////////////////
	// Melee

	void MeleeAttack();

	//////////////////////////////////////////////////////////////////////////
	// Damage

	UPROPERTY(EditAnywhere, Category = Damage)
	float PercentDamageArmsAndLegs;

	virtual void DealDamage(float Damage, class AZanshinBasicArrow* Arrow);

	UPROPERTY(EditAnywhere,replicated, BlueprintReadWrite, Category = Jump)
	bool bCanDieNextArrow;

	UPROPERTY(EditAnywhere, replicated, BlueprintReadWrite, Category = Jump)
	bool bIsSlowed;

	void ActiveNextArrowKill();
	void DeactivatedNextArrowKill();
	FTimerHandle TimerHandle_DeactivatedNextArrowKill;

	void ActiveIsSlowed();

	//////////////////////////////////////////////////////////////////////////
	// Jump

	/** Jump height */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Jump)
	float JumpHeight;

	UPROPERTY(EditAnywhere, Category = Jump)
	float JumpAirControl;

	/** JetJump height */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Jump)
	float JetJumpHeight = 1000.f;

	UPROPERTY(EditAnywhere, Category = Jump)
	float JetJumpAirControl;

	/** Jump delay */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Jump)
	float JumpDelay;

	//////////////////////////////////////////////////////////////////////////
	// Check Self Heal
	UPROPERTY(EditAnywhere, Replicated, BlueprintReadWrite, Category = Heal)
	bool bSelfHeal;

	//////////////////////////////////////////////////////////////////////////
	// Check Slef Hit Feedback

	UPROPERTY(EditAnywhere,Replicated, BlueprintReadWrite, Category = Hit)
	bool bSelfHit;

	UFUNCTION(BlueprintCallable, Category = Enemy)
	void ResetSelfHit();

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Enemy)
	FTimerHandle TimerHandle_ResetSelfHit;

	//Impulse If The Player Has Die
	UFUNCTION(BlueprintCallable, Category = Enemy)
	virtual void ImpulseOnBone(float ImpulseApplied, const FHitResult& Hit, const FVector ArrowDirection);

	//////////////////////////////////////////////////////////////////////////
	// Check Enemy Hit Feedback

	/*Ulises*/
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Enemy)
	float PokeStrength;

	UPROPERTY(EditAnywhere, Replicated, BlueprintReadWrite, Category = Enemy)
	bool bEnemyHit;

	UFUNCTION(BlueprintCallable, Category = Enemy)
	void ResetEnemyHit();

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Enemy)
	FTimerHandle TimerHandle_ResetEnemyHit;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Enemy)
	float TimeBetweenPlayerHit;

	////////////////////////////////////////////////////////////////////////
	///// Raycastingi

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Raycast)
	bool bEnemyOnSight;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Raycast)
	bool bAllyOnSight;

	//////////////////////////////////////////////////////////////////////////
	// Run

	/** Running speed.*/
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Gameplay)
	float RunnigSpeed;

	/** Max Stamina.*/
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Gameplay)
	float MaxStamina;

	/** Current Stamina.*/
	UPROPERTY(EditAnywhere, replicated, BlueprintReadWrite, Category = Gameplay)
	float CurrentStamina;

	/** How much stamina will be consume.*/
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Gameplay)
	float StaminaConsume;

	/** How much the player need to have(stamina) before can run again. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Gameplay)
	float StaminaThreshold;

	/** get running state */
	UFUNCTION(BlueprintCallable, Category = Pawn)
	bool IsRunning() const;

	UPROPERTY(replicated)
	bool bIsShooting;

	UPROPERTY(EditAnywhere, replicated, BlueprintReadWrite, Category = Gameplay)
	bool bCanShoot;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Gameplay)
	float TimeBackNormalSpeed;

	void SlowDown(float Amount);

	//////////////////////////////////////////////////////////////////////////
	// Particle on Character

	/** The Particle that will be played when the character suffers an effect from an arrow. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Particle)
	class UParticleSystemComponent* ParticleOnCharacter;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Particle)
	class UParticleSystem* BloodParticle;


	//////////////////////////////////////////////////////////////////////////
	// Set Particle on Character
	UFUNCTION(BlueprintCallable, Category = "Effects")
	void SetParticle(UParticleSystem* ParticleFromArrow);

	UFUNCTION(BlueprintCallable, Category = "Effects")
	void EraseParticle();

	FTimerHandle TimerHandle_EraseParticle;

	UPROPERTY(EditAnywhere,BlueprintReadWrite, Category = "Effects")
	float TimeToResetParticle;

	//////////////////////////////////////////////////////////////////////////
	// Getter and Setter

	USkeletalMeshComponent* GetPawnMesh() const;

	///** Returns Mesh1P subobject **/
	FORCEINLINE class USkeletalMeshComponent* GetSpecifcPawnMesh(bool WantFirstPerson) const { return WantFirstPerson == true ? MeshArms : GetMesh(); }

	/** Returns FirstPersonCameraComponent subobject **/
	FORCEINLINE class UCameraComponent* GetFirstPersonCameraComponent() const { return FirstPersonCameraComponent; }

	/** Get the equiped bow. */
	UFUNCTION(BlueprintCallable, Category = "Bow")
	class AZanshinBow* GetBow() const;

	/** get aiming state */
	UFUNCTION(BlueprintCallable, Category = Team)
	int32 GetTeam() const;
	
	/** get weapon attach point */
	FName GetFirstPersonBowAttachPoint() const;
	FName GetThirdPersonBowAttachPoint() const;

	/** get aiming state */
	UFUNCTION(BlueprintCallable, Category = Gameplay)
	bool IsAiming() const;

	/** get running state */
	UFUNCTION(BlueprintCallable, Category = Gameplay)
	bool IsbRunning() const;

	UFUNCTION(BlueprintCallable, Category = Gameplay)
	void SetIsAiming(bool NewIsAiming);

	/** get reloading state */
	UFUNCTION(BlueprintCallable, Category = Gameplay)
	bool IsReloading() const;
	/** set reloading state */
	UFUNCTION(BlueprintCallable, Category = Gameplay)
	void SetIsReloading(bool NewIsReloading);

	/** get melee attack state */
	UFUNCTION(BlueprintCallable, Category = Gameplay)
	bool IsMeleeAttack() const;

	/** set melee attack state */
	UFUNCTION(BlueprintCallable, Category = Gameplay)
	void SetIsMeleeAttack(bool NewIsMeleeAttack);

	/** get canceling state */
	UFUNCTION(BlueprintCallable, Category = Gameplay)
	bool IsCancelling() const;
	
	/** set canceling state */
	UFUNCTION(BlueprintCallable, Category = Gameplay)
	void SetIsCancelling(bool NewIsCancelling);	

	UFUNCTION(BlueprintCallable, Category = Gameplay)
	bool IsChangingArrow() const;
	
	UFUNCTION(BlueprintCallable, Category = Gameplay)
	void SetIsChangingArrow(bool NewIsChangingArrow);

protected:

	// APawn interface
	virtual void BeginPlay() override;
	virtual void SetupPlayerInputComponent(UInputComponent* InputComponent) override;
	virtual void Tick(float DeltaSeconds) override;
	/** spawn inventory, setup initial variables */
	virtual void PostInitializeComponents() override;
	// End of APawn interface
	
	//////////////////////////////////////////////////////////////////////////
	// Arrow

	int32 ChangeArrowAmmoZeroIndex;

	void ChangeToDefaultArrowIfNoAmmo();

	void CheckIfTheNextArrowHasAmmo(int32 Direction);

	//////////////////////////////////////////////////////////////////////////
	// Movement

	/** Handles moving forward/backward */
	void MoveForward(float Val);

	/** Handles stafing movement, left and right */
	void MoveRight(float Val);

	void StartCrouch();

	void StopCrouch();

	bool bCanCrouch;

	void BackToNormalSpeed();

	UPROPERTY(replicated)
	float SpeedAmount = 1.f;

	//////////////////////////////////////////////////////////////////////////
	// Run

	void ControlStamina(float DeltaTime);
	void Run();
	void StopRun();

	//////////////////////////////////////////////////////////////////////////
	// Jump

	void OnJump();
	void OnJetJump();

	//////////////////////////////////////////////////////////////////////////
	// Aim Mouse

	void TurnAtRateMouse(float Rate);

	void LookUpAtRateMouse(float Rate);

	//////////////////////////////////////////////////////////////////////////
	// Aim Controller

	/**
	* Called via input to turn at a given rate.
	* @param Rate	This is a normalized rate, i.e. 1.0 means 100% of desired turn rate
	*/
	void TurnAtRate(float Rate);

	/**
	* Called via input to turn look up/down at a given rate.
	* @param Rate	This is a normalized rate, i.e. 1.0 means 100% of desired turn rate
	*/
	void LookUpAtRate(float Rate);

	//////////////////////////////////////////////////////////////////////////
	// Damage

	FVector DirectionOfTheArrow;

	UPROPERTY()
	TArray<AZanshinCharacter*> KillerAssist;

	UFUNCTION(BlueprintCallable, Category = "Damage")
	void OnShot(class AActor* OtherActor, class UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit);

	UFUNCTION(BlueprintCallable, Category = "Damage")
	void OnHeadShot(class AActor* OtherActor, class UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit);

	UFUNCTION(BlueprintCallable, Category = "Damage")
	void OnChestShot(class AActor* OtherActor, class UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit);

	UFUNCTION(BlueprintCallable, Category = "Damage")
	void OnArmsAndLegsShot(class AActor* OtherActor, class UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit);

	/** Take damage, handle death */
	virtual float TakeDamage(float Damage, struct FDamageEvent const& DamageEvent, class AController* EventInstigator, class AActor* DamageCauser) override;

	virtual bool Die(float KillingDamage, struct FDamageEvent const& DamageEvent, class AController* Killer, class AActor* DamageCauser);
	
	/** Check to see if the player is from your team */
	virtual bool CheckToSeeIfItsEnemy(class AZanshinBasicArrow* Arrow);

	//////////////////////////////////////////////////////////////////////////
	// Bow
	
	/** socket or bone name for attaching weapon mesh */
	UPROPERTY(EditAnywhere, Category = Gameplay)
	FName ThirdPersonBowAttachPoint;

	UPROPERTY(EditAnywhere, Category = Gameplay)
	FName FirstPersonBowAttachPoint;

	//////////////////////////////////////////////////////////////////////////
	// Ragdoll

	void ActiveRagdoll();
	void DisableRagdoll();
	void AddForceToRagdoll(FVector Direction);

	UPROPERTY(EditAnywhere, BlueprintReadWrite, replicated, Category = Run)
	bool bIsRunning;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, replicated, Category = Moving)
	bool bIsMovingForward;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, replicated, Category = Moving)
	bool bIsMovingRight;

	//////////////////////////////////////////////////////////////////////////
	// Change Arrows

	void ArrowOne();
	void ArrowTwo();
	void ArrowThree();
	void ArrowFour();
	void ArrowFive();
	void ChangeArrowFromKey(int32 ArrowIndexKey);

private:

	//////////////////////////////////////////////////////////////////////////
	// Bow
	UPROPERTY(replicated)
	class AZanshinBow* Bow;

	//////////////////////////////////////////////////////////////////////////
	// Aim

	UPROPERTY(EditAnywhere, replicated, Category = Aim)
	bool bIsAiming;

	//////////////////////////////////////////////////////////////////////////
	// Reload

	UPROPERTY(EditAnywhere, replicated, Category = Reload)
	bool bIsReloading;

	void SetCanShoot();

	void ShowFirstArrow();

	//////////////////////////////////////////////////////////////////////////
	// Melee

	UPROPERTY(EditAnywhere, replicated, Category = Reload)
	bool bIsMeleeAttack;

	UPROPERTY(EditAnywhere, replicated, Category = Achievement)
	bool bIsNormalKill;

	UPROPERTY(EditAnywhere, replicated, Category = Achievement)
	bool bIsHeadShotKill;

	UPROPERTY(EditAnywhere, replicated, Category = Achievement)
	bool bIsMeleeKill;

	

	KillType CurrentKillType;
	ArrowKillType CurrentArrowKillType;

	//////////////////////////////////////////////////////////////////////////
	// Cancelling

	UPROPERTY(EditAnywhere, replicated, Category = Reload)
	bool bIsCancelling;

	//////////////////////////////////////////////////////////////////////////
	// Jump

	UPROPERTY(EditAnywhere, replicated, Category = Jump)
	bool bIsJetJumping;

	//////////////////////////////////////////////////////////////////////////
	// Run	

	UPROPERTY(EditAnywhere, replicated, Category = Run)
	float MaxWalkSpeedDefault;

	UPROPERTY(EditAnywhere, replicated, Category = Run)
	float ShootWalkSpeed;

	//////////////////////////////////////////////////////////////////////////
	// Damage

	TArray<AZanshinBasicArrow*> ArrowStuckOnYou;

	void DestroyArrowsStuckOnYou();

	//////////////////////////////////////////////////////////////////////////
	// Reset Collision

	void NoMeleeCollision();

	//////////////////////////////////////////////////////////////////////////
	// Reset AllowJetJump

	void CancelAllowJetJump();

	//////////////////////////////////////////////////////////////////////////
	// Timer Handles

	UPROPERTY(EditAnywhere, Category = Jump)
	FTimerHandle JumpTimerHandle;

	UPROPERTY(EditAnywhere, Category = Jump)
	FTimerHandle MeleeTimerHandle;

	UPROPERTY(EditAnywhere, Category = Jump)
	FTimerHandle ReloadTimerHandle;

	/** Handle the timer to destroy the components attaches to the player(destroy the arrow stucked on the player) */
	FTimerHandle TimerHandle_DestroyComponentsAttaches;

	/** Handle the timer to go back to normal speed */
	FTimerHandle TimerHandle_GoBackToNormalSpeed;

	FTimerHandle TimerHandle_ShowFirstArrow;

	FTimerHandle TimerHandle_AllowToJetJump;

public:

	////////////////////////////////////////////////////////////////////////
	///// Networking 
	
	/** [Server] Handles the logic of firing. */
	void Fire();

	/** Called from the clients to shoot on the server. 
	*
	*	@param	ClientSpawnLocation	The location where the arrow will spawn.
	*	@param	ClientSpawnRotation	The rotation at which the arrow will spawn.
	*/
	UFUNCTION(reliable, server, WithValidation)
	void ServerFire(FVector ClientSpawnLocation, FRotator ClientSpawnRotation);
	virtual void ServerFire_Implementation(FVector ClientSpawnLocation, FRotator ClientSpawnRotation);
	virtual bool ServerFire_Validate(FVector ClientSpawnLocation, FRotator ClientSpawnRotation);

	/** [Server] Called on clients when they pull the string. Add client logic here. */
	void PullString();

	/** Informs the UI about changes in Arrow Power. */
	UFUNCTION(BlueprintImplementableEvent, meta = (DisplayName = "UpdateHUDIndicators"))
	void UpdateHUDIndicators(bool bIsChargingPower);

	/** Called when the client pulls the string. To handle power update on the server. */
	UFUNCTION(reliable, server, WithValidation)
	void ServerPullString();
	virtual void ServerPullString_Implementation();
	virtual bool ServerPullString_Validate();

	/** Jet Jump on the server. */
	UFUNCTION(reliable, server, WithValidation)
	void Server_OnJetJump();
	virtual void Server_OnJetJump_Implementation();
	virtual bool Server_OnJetJump_Validate();

	/** Normal Jump on the server. */
	UFUNCTION(reliable, server, WithValidation)
	void ServerOnJump();
	virtual void ServerOnJump_Implementation();
	virtual bool ServerOnJump_Validate();

	/** Called from the server to run on all clients. Sets the JumpHeight and the Air Control
	*
	*	@param	CurrentJumpHeight	The Jump Height of the next jump.
	*	@param	AirControl			The air control of the next jump.
	*/
	void SetJumpHeight(float CurrentJumpHeight, float AirControl);

	/** Called from Server. Sets JumpHeight on all clients. */
	UFUNCTION(NetMulticast, Reliable, WithValidation)
	void NetMulticastSetJumpHeight(float CurrentJumpHeight, float AirControl);
	virtual bool NetMulticastSetJumpHeight_Validate(float CurrentJumpHeight, float AirControl);
	virtual void NetMulticastSetJumpHeight_Implementation(float CurrentJumpHeight, float AirControl);

	/** Called from Clients. Tells the server to update walking speed. */
	UFUNCTION(reliable, server, WithValidation)
	void ServerRun();
	virtual void ServerRun_Implementation();
	virtual bool ServerRun_Validate();

	/** Called from Clients. Tells the server to update walking speed. */
	UFUNCTION(reliable, server, WithValidation)
	void ServerStopRun();
	virtual void ServerStopRun_Implementation();
	virtual bool ServerStopRun_Validate();

	/** Called from Clients. Tells the server to update walking speed. */
	UFUNCTION(NetMulticast, Reliable, WithValidation)
	void NetMulticastSetWalkSpeed(float CurrentWalkSpeed);
	virtual bool NetMulticastSetWalkSpeed_Validate(float CurrentWalkSpeed);
	virtual void NetMulticastSetWalkSpeed_Implementation(float CurrentWalkSpeed);

	/** Called from the server. Plays a sound at the current location of the client. */
	UFUNCTION(client, reliable, withvalidation)
	void Client_PlaySound(USoundBase* DesiredSound);
	bool Client_PlaySound_Validate(USoundBase* DesiredSound);
	void Client_PlaySound_Implementation(USoundBase* DesiredSound);

	/** Called from the server. Plays a sound at the current location of the client. */
	UFUNCTION(client, reliable, withvalidation)
	void Client_PlaySound_Attached(USoundBase* DesiredSound);
	bool Client_PlaySound_Attached_Validate(USoundBase* DesiredSound);
	void Client_PlaySound_Attached_Implementation(USoundBase* DesiredSound);

	/** Triggers ragdoll effect on all clients. */
	UFUNCTION(NetMulticast, Reliable, WithValidation)
	void NetMulticastHandleRagdollEffect();
	virtual bool NetMulticastHandleRagdollEffect_Validate();
	virtual void NetMulticastHandleRagdollEffect_Implementation();

	/** Add force to all the ragdolls. */
	UFUNCTION(NetMulticast, Reliable, WithValidation)
	void NetMulticastAddForceToRagdoll(FVector Direction);
	virtual bool NetMulticastAddForceToRagdoll_Validate(FVector Direction);
	virtual void NetMulticastAddForceToRagdoll_Implementation(FVector Direction);

	/** Change arrow on the server. */
	UFUNCTION(server, reliable, withvalidation)
	void ServerNextArrow();
	virtual void ServerNextArrow_Implementation();
	virtual bool ServerNextArrow_Validate();

	UFUNCTION(server, reliable, withvalidation)
		void Server_ChangeArrowFromKey(int32 ArrowIndexKey);
	virtual void Server_ChangeArrowFromKey_Implementation(int32 ArrowIndexKey);
	virtual bool Server_ChangeArrowFromKey_Validate(int32 ArrowIndexKey);
	
	/** Change arrow on the server. */
	UFUNCTION(BlueprintCallable, Category = Gameplay)
	void ArrowChangedToHUDTrue();

	/** Change arrow on the server. */
	UFUNCTION(server, reliable, withvalidation)
	void ServerPreviousArrow();
	virtual void ServerPreviousArrow_Implementation();
	virtual bool ServerPreviousArrow_Validate();

	/** Arrow changed on server. */
	UFUNCTION(server, reliable, withvalidation)
	void Server_SetArrowChanged();
	virtual void Server_SetArrowChanged_Implementation();
	virtual bool Server_SetArrowChanged_Validate();

	UFUNCTION(BlueprintCallable, Category = Gameplay)
	void ArrowChangedToHUDFalse();

	/** Reset arrow change to false. */
	UFUNCTION(server, reliable, withvalidation)
	void Server_ResetArrowChanged();
	virtual void Server_ResetArrowChanged_Implementation();
	virtual bool Server_ResetArrowChanged_Validate();

	UPROPERTY(EditAnywhere, Category = Jump)
	FTimerHandle TimerHandle_ArrowToHUD;

	UPROPERTY(EditAnywhere, replicated, BlueprintReadWrite, Category = Inventory)
	bool bArrowChanged;

	float changingArrowTimer;

	UPROPERTY(replicated)
	bool bIsSwitchingArrows = false;

	FTimerHandle TimerHandle_ChangingArrows;

	/** [Clients] Do the actual ragdoll effect. Runs on all clients. */
	void DoRagdoll();

	/** [Clients] Set the walking speed. */
	void SetWalkSpeed(float WalkSpeed);

	UFUNCTION(BlueprintCallable, Category = "Arrow Check")
	void RestrainSwitching();

	/** Check if a client is locally controlled. */
	UFUNCTION(BlueprintCallable, Category = "Bow Check")
	bool IsFirstPerson() const;

	/** Create the animation arrow. */
	UFUNCTION(BlueprintCallable, Category = "Animation")
	void CreateAnimationArrow();

	/** Create the animation arrow. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Arrow)
	AZanshinBasicArrow* AnimationArrow;

	/** The name of the socket where the arrow must go. */
	UPROPERTY(EditAnywhere, Category = Gameplay)
	FName ArrowSocket;

	/** Tell the server that it's reloading. Used in the UI. */
	UFUNCTION(server, reliable, withvalidation)
		void ServerSetIsReloading(bool bNewIsReloading);
	virtual void ServerSetIsReloading_Implementation(bool bNewIsReloading);
	virtual bool ServerSetIsReloading_Validate(bool bNewIsReloading);

	UFUNCTION(server, reliable, withvalidation)
		void ServerSetIsChangingArrow(bool bNewIsChangingArrow);
	virtual void ServerSetIsChangingArrow_Implementation(bool bNewIsChangingArrow);
	virtual bool ServerSetIsChangingArrow_Validate(bool bNewIsChangingArrow);

	void DetachCamera();

	UPROPERTY(replicated)
	int32 ArrowIndex = 0;

	int32 GetArrowIndex();

	void SetupInfo();

	/** Cancel shot on the server. */
	UFUNCTION(server, reliable, withvalidation)
		void Server_CancelShot();
	virtual void Server_CancelShot_Implementation();
	virtual bool Server_CancelShot_Validate();

	/** Updates vector parameter in the material. Using Blueprints. */
	UFUNCTION(BlueprintImplementableEvent, meta = (DisplayName = "UpdateMaterialColor"))
	void UpdateMaterial(int32 InPlayerTeam);

	/** Updates vector parameter in the material. Using Blueprints. */
	UFUNCTION(BlueprintImplementableEvent, meta = (DisplayName = "UpdateTextRender"))
	void UpdateTextRender(FName InPlayerName, int32 InPlayerTeam);

	/** Update the amount of ammo for the current arrow. */
	UFUNCTION(server, reliable, withvalidation)
		void Server_SetCurrentAmmo();
	virtual void Server_SetCurrentAmmo_Implementation();
	virtual bool Server_SetCurrentAmmo_Validate();

	/** Update the amount of max ammo for the current arrow. */
	UFUNCTION(server, reliable, withvalidation)
		void Server_SetCurrentMaxAmmo();
	virtual void Server_SetCurrentMaxAmmo_Implementation();
	virtual bool Server_SetCurrentMaxAmmo_Validate();

	/** Used to add the player feedback when a client dies. */
	UFUNCTION(client, reliable, withvalidation)
		void Client_AddPlayerFeedback_Death();
	bool Client_AddPlayerFeedback_Death_Validate();
	void Client_AddPlayerFeedback_Death_Implementation();

	/** Used to add the player feedback when a client receives a hit. */
	UFUNCTION(client, reliable, withvalidation)
		void Client_AddPlayerFeedback_ReceiveHit();
	bool Client_AddPlayerFeedback_ReceiveHit_Validate();
	void Client_AddPlayerFeedback_ReceiveHit_Implementation();

	/** Used to add the player feedback when a client hits an enemy. */
	UFUNCTION(client, reliable, withvalidation)
		void Client_AddPlayerFeedback_HitEnemy();
	bool Client_AddPlayerFeedback_HitEnemy_Validate();
	void Client_AddPlayerFeedback_HitEnemy_Implementation();

	/** Perform a melee attack on the server. */
	UFUNCTION(server, reliable, withvalidation)
		void Server_MeleeAttack();
	void Server_MeleeAttack_Implementation();
	bool Server_MeleeAttack_Validate();

	UFUNCTION(server, reliable, withvalidation)
		void Server_SetIsMeleeAttack(bool NewIsMeleeAttack);
	void AZanshinCharacter::Server_SetIsMeleeAttack_Implementation(bool NewIsMeleeAttack);
	bool AZanshinCharacter::Server_SetIsMeleeAttack_Validate(bool NewIsMeleeAttack);

	/** [Client] Death Feedback. */
	void AddPlayerFeedBack_Death();

	/** [Client] Receive damage feedback. */
	void AddPlayerFeedback_ReceiveHit();

	/** [Client] Hit an enemy feedback. */
	void AddPlayerFeedback_HitEnemy();

	/** Used to add the player feedback when a client hits an enemy. */
	UFUNCTION(client, reliable, withvalidation)
	void Client_UpdateKillType(KillType Kill);
	bool Client_UpdateKillType_Validate(KillType Kill);
	void Client_UpdateKillType_Implementation(KillType Kill);

	bool ColorUpdated = false;

	void HandleColorChange();

	static int EditorTeamNumber;

	/** Add force to all the ragdolls. */
	UFUNCTION(NetMulticast, Reliable, WithValidation)
		void NetMulticast_ImpulseOnBone(float ImpulseApplied, const FHitResult& Hit, const FVector ArrowDirection);
	bool NetMulticast_ImpulseOnBone_Validate(float ImpulseApplied, const FHitResult& Hit, const FVector ArrowDirection);
	void NetMulticast_ImpulseOnBone_Implementation(float ImpulseApplied, const FHitResult& Hit, const FVector ArrowDirection);

	/** Add force to all the ragdolls. */
	UFUNCTION(NetMulticast, Reliable, WithValidation)
		void NetMulticast_DisableRagdoll();
	bool NetMulticast_DisableRagdoll_Validate();
	void NetMulticast_DisableRagdoll_Implementation();

	UPROPERTY(BlueprintReadWrite, Category = Ragdoll)
	bool bRagdollDisabled = false;

	UFUNCTION(client, reliable, withvalidation)
	void Client_PlaySound_Spawned(USoundBase* DesiredSound, FVector Location, float pitchMultiplier);
	bool Client_PlaySound_Spawned_Validate(USoundBase* DesiredSound, FVector Location, float pitchMultiplier);
	void Client_PlaySound_Spawned_Implementation(USoundBase* DesiredSound, FVector Location, float pitchMultiplier);

	UAudioComponent* HeartBeatSound;
	float HearBeatTreshold = 25.f;

	///////////////////////////////////////////////////////
	//// CONTROL POINTS
	UFUNCTION(BlueprintImplementableEvent, meta = (displayname = "ShowControlPointFeedback"))
	void BP_Event_ShowCPFeedback(float CurrentPoints, int32 CapturePointLetter);

	UFUNCTION(BlueprintImplementableEvent, meta = (displayname = "HideControlPointFeedback"))
	void BP_Event_HideCPFeedback();

	UFUNCTION(client, reliable, withvalidation)
		void Client_UpdateControlPoints(bool bIsInsideCP, float CurrentPoints, int32 CapturePointLetter);
	bool Client_UpdateControlPoints_Validate(bool bIsInsideCP, float CurrentPoints, int32 CapturePointLetter);
	void Client_UpdateControlPoints_Implementation(bool bIsInsideCP, float CurrentPoints, int32 CapturePointLetter);

};