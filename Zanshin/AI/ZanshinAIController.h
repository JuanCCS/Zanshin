// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "ZanshinBot.h"
#include "ZanshinDrone.h"
#include "AIController.h"
#include "BehaviorTree/BehaviorTree.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "BehaviorTree/BehaviorTreeComponent.h"
#include "BehaviorTree/Blackboard/BlackboardKeyType_Vector.h"
#include "ZanshinAIController.generated.h"

/**
*
*/
UCLASS()
class ZANSHIN_API AZanshinAIController : public AAIController
{
	GENERATED_BODY()

	//////////////////////////////////////////////////////////////////////////
	// Components

	UPROPERTY(transient)
	class UBlackboardComponent* BlackboardComponent;

	UPROPERTY(transient)
	class UBehaviorTreeComponent* BehaviorTreeComponent;

public:

	//////////////////////////////////////////////////////////////////////////
	// Constructor

	AZanshinAIController(const FObjectInitializer& ObjectInitializer);

	//////////////////////////////////////////////////////////////////////////
	// Basic

	// Called when the game starts or when spawned
	/** after all game elements are created */
	virtual void PostInitializeComponents() override;
	virtual void BeginPlay() override;
	virtual void BeginInactiveState() override;
	virtual void Possess(class APawn* InPawn);

	void Respawn();

	//////////////////////////////////////////////////////////////////////////
	// Navigation

	UPROPERTY(EditAnywhere, Category = Gameplay)
	TArray<ATargetPoint*> LocationsToWalk;

	UFUNCTION(BlueprintCallable, Category = Behavior)
	void FindNextPathNode();

	//////////////////////////////////////////////////////////////////////////
	// Fire

	UFUNCTION(BlueprintCallable, Category = Gameplay)
	void OnPullString();
	void OnFire();

	////////////////////////////////////////////
	/// Team

	/** The team for each character **/
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Gameplay)
	int32 CharacterTeam;

	//////////////////////////////////////////////////////////////////////////
	// Getters and Setters

	FORCEINLINE uint8 GetPathIndex() const { return PathIndex; }
	FORCEINLINE void SetPathIndex(uint8 NewIndex) { PathIndex = NewIndex; }

protected:
	/** Handle for efficient management of Respawn timer */
	FTimerHandle TimerHandle_Respawn;

private:

	//////////////////////////////////////////////////////////////////////////
	// BehaviorTree Keys

	uint8 LocationID;

	//////////////////////////////////////////////////////////////////////////
	// Navigation

	uint8 PathIndex;

	void SetLocation();

	//////////////////////////////////////////////////////////////////////////
	// Fire

	UPROPERTY(EditAnywhere, Category = Gameplay)
	float PullStringTime;

	//////////////////////////////////////////////////////////////////////////
	// Timer Handles

	FTimerHandle PullStringTimerHandle;
};
