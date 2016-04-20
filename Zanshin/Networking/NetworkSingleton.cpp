// Fill out your copyright notice in the Description page of Project Settings.

#include "Zanshin.h"
#include "Engine.h"
#include "NetworkSingleton.h"

// Sets default values
ANetworkSingleton::ANetworkSingleton()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
}

// Called when the game starts or when spawned
void ANetworkSingleton::BeginPlay()
{
	Super::BeginPlay();
	
}

// Called every frame
void ANetworkSingleton::Tick( float DeltaTime )
{
	Super::Tick( DeltaTime );

}

//Singleton implementation.
ANetworkSingleton& ANetworkSingleton::GetInstance()
{
	static ANetworkSingleton instance;
	return instance;
}