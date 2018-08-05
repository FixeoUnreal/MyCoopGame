// Fill out your copyright notice in the Description page of Project Settings.

using UnrealBuildTool;
using System.Collections.Generic;

public class CoopGameTarget : TargetRules
{
	public CoopGameTarget(TargetInfo Target) : base(Target)
	{
		Type = TargetType.Game;

		ExtraModuleNames.AddRange( new string[] { "CoopGame" } );

        //Non-Unity-mode
        bUseUnityBuild = false;

        //New standard
        bUsePCHFiles = false;
    }
}
