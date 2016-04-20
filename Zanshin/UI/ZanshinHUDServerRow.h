// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "Blueprint/UserWidget.h"
#include "ZanshinHUDServerRow.generated.h"

/**
 * 
 */
UCLASS()
class ZANSHIN_API UZanshinHUDServerRow : public UUserWidget
{
	GENERATED_BODY()
	
		FOnlineSessionSearchResult RowResult;

public:

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "WIDGETS")
		FString RowServerName;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "WIDGETS")
		FString RowPlayers;

	UFUNCTION(BlueprintCallable, Category = "ClickEvents")
		void RowClickEvent();

	void SetServerName(FString InstanceServerName);

	void SetPlayers(FString InstancePlayers);

	void SetSearchResult(FOnlineSessionSearchResult RowSearchResult);
	
	
};
