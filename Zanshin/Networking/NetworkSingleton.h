// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "GameFramework/Actor.h"
#include "NetworkSingleton.generated.h"

UCLASS()
class ZANSHIN_API ANetworkSingleton : public AActor
{
	GENERATED_BODY()
	
public:	

	/** Returns the instance, creates it if it doesn't exist. */
	static ANetworkSingleton& GetInstance();

	/** Sets default values for this actor's properties */
	ANetworkSingleton();

	/** Sets default values for this actor's properties */
	virtual void BeginPlay() override;
	
	/** Sets default values for this actor's properties */
	virtual void Tick( float DeltaSeconds ) override;

private:

};
