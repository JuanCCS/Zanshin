// Fill out your copyright notice in the Description page of Project Settings.

#include "Zanshin.h"
#include "NetworkDebugInterface.h"


UNetworkDebugInterface::UNetworkDebugInterface(const class FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer)
{ }

void INetworkDebugInterface::WriteToNetLog(FString DebugMessage, UWorld* CurrentWorld)
{
	if (GEngine)
	{
		if (GEngine->GetNetMode(CurrentWorld) == ENetMode::NM_Client)
		{
			// set this one to log so it shows grey
			UE_LOG(Networking, Log, TEXT("INTERFACE_CLIENT - %s"), *DebugMessage);
		}
		else if (GEngine->GetNetMode(CurrentWorld) == ENetMode::NM_ListenServer)
		{
			// set this one to error so it shows red.
			UE_LOG(Networking, Error, TEXT("INTERFACE_LISTEN_SERVER - %s"), *DebugMessage);
		}
		else if (GEngine->GetNetMode(CurrentWorld) == ENetMode::NM_DedicatedServer)
		{
			// set this one to error so it shows red.
			UE_LOG(Networking, Error, TEXT("INTERFACE_DEDICATED_SERVER - %s"), *DebugMessage);
		}
	}
}

//Raises errors sometimes, use it when absolutely necesary.
void INetworkDebugInterface::DebugNetworkState(FString DebugMessage, UWorld* CurrentWorld, AActor* Actor)
{
	if (GEngine)
	{
		if (GEngine->GetNetMode(CurrentWorld) == ENetMode::NM_Client)
		{
			// set this one to log so it shows grey
			UE_LOG(Networking, Log, TEXT("INTERFACE_CLIENT - %s"), *DebugMessage);
		}
		else if (GEngine->GetNetMode(CurrentWorld) == ENetMode::NM_ListenServer)
		{
			// set this one to error so it shows red.
			UE_LOG(Networking, Error, TEXT("INTERFACE_LISTEN_SERVER - %s"), *DebugMessage);
		}
		else if (GEngine->GetNetMode(CurrentWorld) == ENetMode::NM_DedicatedServer)
		{
			// set this one to error so it shows red.
			UE_LOG(Networking, Error, TEXT("INTERFACE_DEDICATED_SERVER - %s"), *DebugMessage);
		}
			if (!Actor->GetOwner()->GetName().IsEmpty()){
				UE_LOG(Networking, Log, TEXT("Owner - %s"), *Actor->GetOwner()->GetName());
			}
			if (!Actor->GetName().IsEmpty()){
				UE_LOG(Networking, Log, TEXT("Name - %s"), *Actor->GetName());
			}
	}
}