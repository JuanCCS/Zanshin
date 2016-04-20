// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "GameFramework/Character.h"
#include "Networking/NetworkDebugInterface.h"
#include "ZanshinNetRepCharacter.generated.h"

UCLASS()
class ZANSHIN_API AZanshinNetRepCharacter : public ACharacter, public INetworkDebugInterface
{
	GENERATED_BODY()

public:

	/** User Chosen net priority value. */ 
	UPROPERTY( EditAnywhere, BlueprintReadWrite, Category = Network )
	float DesiredNetPriority;

	/** User Chosen net update frequency (Hz). */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Network)
	float DesiredNetUpdateFrequency;


	// Sets default values for this character's properties
	AZanshinNetRepCharacter(const class FObjectInitializer& ObjectInitializer);

	// Called when the game starts or when spawned
	virtual void BeginPlay() override;
	
	// Called every frame
	virtual void Tick( float DeltaSeconds ) override;

	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* InputComponent) override;

	///////////////////////////////////////////////////
	/// NETWORKING

	UFUNCTION(Server, Reliable, WithValidation)
	void ROS_Function();
	bool ROS_Function_Validate();
	void ROS_Function_Implementation();
};
