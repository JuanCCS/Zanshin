// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "GameFramework/SpectatorPawn.h"
#include "ZanshinSpectatorPawn.generated.h"

/**
 * 
 */
UCLASS(config = Game, Blueprintable, BlueprintType)
class ZANSHIN_API AZanshinSpectatorPawn : public ASpectatorPawn
{
	GENERATED_UCLASS_BODY()
	
	//////////////////////////////////////////////////////////////////////////
	// Components

	/** Spring Arm Component*/
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = SpringArm, meta = (AllowPrivateAccess = "true"))
	class USpringArmComponent* SpringArmComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
	class UCameraComponent* FirstPersonCameraComponent;

	/** Overridden to implement Key Bindings the match the player controls */
	virtual void SetupPlayerInputComponent(class UInputComponent* InputComponent) override;
	// End Pawn overrides

	// Frame rate linked look
	void LookUpAtRate(float Val);

	void LowSpeed();
	void HightSpeed();
};