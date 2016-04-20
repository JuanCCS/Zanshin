// Copyright 1998-2015 Epic Games, Inc. All Rights Reserved.

#include "Zanshin.h"
#include "Engine.h"


IMPLEMENT_PRIMARY_GAME_MODULE( FDefaultGameModuleImpl, Zanshin, "Zanshin" );

//General Log
DEFINE_LOG_CATEGORY(Server);

//Logging during game startup
DEFINE_LOG_CATEGORY(Client);

//Logging for your AI system
DEFINE_LOG_CATEGORY(Networking);

//Logging for Critical Errors that must always be addressed
DEFINE_LOG_CATEGORY(Physics);

   