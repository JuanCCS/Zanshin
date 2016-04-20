// Copyright 1998-2015 Epic Games, Inc. All Rights Reserved.

#ifndef __ZANSHIN_H__
#define __ZANSHIN_H__

/* Stencil index mapping to PP_OutlineColored */
#define STENCIL_FRIENDLY_OUTLINE 252;
#define STENCIL_NEUTRAL_OUTLINE 253;
#define STENCIL_ENEMY_OUTLINE 254;
#define STENCIL_ITEMHIGHLIGHT 255;

#include "Engine.h"
#include "UnrealNetwork.h"
#include "Runtime/UMG/Public/UMG.h"
#include "Runtime/UMG/Public/UMGStyle.h"
#include "Runtime/UMG/Public/Slate/SObjectWidget.h"
#include "Runtime/UMG/Public/IUMGModule.h"
#include "Runtime/UMG/Public/Blueprint/UserWidget.h"
#include "Online.h"

//Includes from the game.
#include "Player/ZanshinCharacter.h"
#include "Bow/ZanshinBow.h"
#include "Arrow/ZanshinBasicArrow.h"

//General Log
DECLARE_LOG_CATEGORY_EXTERN(Server, Log, All);

//Logging during game startup
DECLARE_LOG_CATEGORY_EXTERN(Client, Log, All);

//Logging for your AI system
DECLARE_LOG_CATEGORY_EXTERN(Networking, Log, All);

//Logging for Critical Errors that must always be addressed
DECLARE_LOG_CATEGORY_EXTERN(Physics, Log, All);

#endif