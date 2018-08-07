// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "SWeapon.h"
#include "SGrenadeLauncher.generated.h"

class ASProjectile;

/**
 * 
 */
UCLASS()
class COOPGAME_API ASGrenadeLauncher : public ASWeapon
{
	GENERATED_BODY()
	
protected:
	virtual void Fire() override;
	
	/** Projectile class to spawn */
	UPROPERTY(EditDefaultsOnly, Category = "Projectile")
	TSubclassOf<ASProjectile> ProjectileClass;
	
};
