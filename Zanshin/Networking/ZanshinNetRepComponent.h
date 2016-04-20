// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "Components/ActorComponent.h"
#include "Networking/NetworkDebugInterface.h"
#include "ZanshinNetRepComponent.generated.h"


UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class ZANSHIN_API UZanshinNetRepComponent : public UActorComponent, public INetworkDebugInterface
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	UZanshinNetRepComponent();



	// Called when the game starts
	virtual void BeginPlay() override;
	
	// Called every frame
	virtual void TickComponent( float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction ) override;

		
	
};
