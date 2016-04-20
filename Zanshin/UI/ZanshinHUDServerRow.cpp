// Fill out your copyright notice in the Description page of Project Settings.

#include "Zanshin.h"
#include "ZanshinHUDServerRow.h"


void UZanshinHUDServerRow::RowClickEvent(){
	return;
}

void UZanshinHUDServerRow::SetServerName(FString InstanceServerName){
	RowServerName = InstanceServerName;
	return;
}

void UZanshinHUDServerRow::SetPlayers(FString InstancePlayers){
	RowPlayers = InstancePlayers;
	return;
}

void UZanshinHUDServerRow::SetSearchResult(FOnlineSessionSearchResult RowSearchResult){
	RowResult = RowSearchResult;
	return;
}


