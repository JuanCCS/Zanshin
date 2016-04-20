// Copyright 1998-2015 Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;

public class Zanshin : ModuleRules
{
	public Zanshin(TargetInfo Target)
	{
		PublicDependencyModuleNames.AddRange(new string[] { "Core", "CoreUObject", "Engine", "InputCore", "AIModule", "UMG", "Slate", "SlateCore", "OnlineSubsystem", "OnlineSubsystemUtils"});

        DynamicallyLoadedModuleNames.Add("OnlineSubsystemNull");
	}
}
