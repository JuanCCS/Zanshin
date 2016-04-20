// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "Zanshin.h"
#include "Object.h"
#include "NetworkDebugInterface.generated.h"

/**
 * An Interface to add to Replicated Actors to debug information about function calls on Server and Client.
 */
UINTERFACE(MinimalAPI)
class UNetworkDebugInterface : public UInterface
{
	GENERATED_UINTERFACE_BODY()
};

class INetworkDebugInterface {
	GENERATED_IINTERFACE_BODY()

	/**
	* Log a Message to the Output Log. Indicates wether it's running on the server
	* or clients.
	*
	* @param	DebugMessage The message you want to log.
	* @param	CurrentWorld The World in which the code is running.
	*/
	void WriteToNetLog(FString DebugMessage, UWorld* CurrentWorld);
	
	/**
	* Logs DebugMessage to the Output Log. As well as the Owner and Name of the Actor.
	*
	* @param	DebugMessage	The message you want to log.
	* @param	CurrentWorld	The World in which the code is running.
	* @param	Actor			The actor being debugged.
	*/
	void DebugNetworkState(FString DebugMessage, UWorld* CurrentWorld, AActor* Actor);

	/**
	* Outputs the string value of an Enum.
	*
	* @param	Name		The message you want to log.
	* @param	Value		The World in which the code is running.
	*/
	template<typename TEnum>
	FString GetEnumValueAsString(const FString& Name, TEnum Value)
	{
		const UEnum* enumPtr = FindObject<UEnum>(ANY_PACKAGE, *Name, true);
		if (!enumPtr)
		{
			return FString("Invalid");
		}

		return enumPtr->GetEnumName((int32)Value);
	}
};
