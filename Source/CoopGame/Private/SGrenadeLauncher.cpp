// Fill out your copyright notice in the Description page of Project Settings.

#include "SGrenadeLauncher.h"
#include "Public/SProjectile.h"
#include "Components/SkeletalMeshComponent.h"



void ASGrenadeLauncher::Fire()
{
	AActor* MyOwner = GetOwner();
	if (!MyOwner) { return; }

	FVector EyeLocation;
	FRotator EyeRotation;
	MyOwner->GetActorEyesViewPoint(EyeLocation, EyeRotation);
	// try and fire a projectile
	if (ProjectileClass)
	{
		FVector MuzzleLocation = MeshComp->GetSocketLocation(MuzzleSocketName);
		FRotator MuzzleRotation = MeshComp->GetSocketRotation(MuzzleSocketName);

		//Set Spawn Collision Handling Override
		FActorSpawnParameters ActorSpawnParams;
		ActorSpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButDontSpawnIfColliding;
		ActorSpawnParams.Instigator = GetOwner()->GetInstigator();

		// spawn the projectile at the muzzle
		GetWorld()->SpawnActor<ASProjectile>(ProjectileClass, MuzzleLocation, EyeRotation, ActorSpawnParams);
	}
}
