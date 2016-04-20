// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "ZanshinSwitchCover.h"
#include "GameFramework/Actor.h"
#include "ZanshinCover.generated.h"

UCLASS()
class ZANSHIN_API AZanshinCover : public AActor
{
	GENERATED_BODY()
	
public:	

	//////////////////////////////////////////////////////////////////////////
	// Constructor

	// Sets default values for this actor's properties
	AZanshinCover();

	/** Store the switches needed to move the cover*/
	UPROPERTY(EditAnywhere, Category = Gameplay)
	TArray<AZanshinSwitchCover*> Switches;

	/** Store the points to move to */
	UPROPERTY(EditAnywhere, Category = Gameplay)
	TArray<ATargetPoint*> Points;

	UPROPERTY(EditAnywhere, Category = Gameplay)
	float MoveSpeedMultiplier;

	// Called when the game starts or when spawned
	virtual void BeginPlay() override;
	
	// Called every frame
	virtual void Tick( float DeltaSeconds ) override;

	void SendClick();

private:

	int ClicksToActivate;
	float MoveLerp;
	bool bCanMove;
	int PointsIndex;
	float AngleToRotate;

	void Move(float DeltaTime);

	void CountSwitches();
};