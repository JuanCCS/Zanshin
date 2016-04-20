// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "GameFramework/Actor.h"
#include "Networking/NetworkDebugInterface.h"
#include "ZanshinNetRepActor.generated.h"

UCLASS()
class ZANSHIN_API AZanshinNetRepActor : public AActor, public INetworkDebugInterface
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AZanshinNetRepActor();

	// Called when the game starts or when spawned
	virtual void BeginPlay() override;
	
	// Called every frame
	virtual void Tick( float DeltaSeconds ) override;

	
	
};
