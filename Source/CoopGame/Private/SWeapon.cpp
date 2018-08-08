// Fill out your copyright notice in the Description page of Project Settings.

#include "SWeapon.h"
#include <DrawDebugHelpers.h>
#include <Kismet/GameplayStatics.h>
#include "Particles/ParticleSystem.h"
#include "Components/SkeletalMeshComponent.h"
#include <Particles/ParticleSystemComponent.h>

static int32 DebugWeaponDrawing = 0;
FAutoConsoleVariableRef CVARDebugWeaponDrawing (
	TEXT("COOP.DebugWeapons"),
	DebugWeaponDrawing,
	TEXT("Draw debug line for weapons"),
	ECVF_Cheat
);

// Sets default values
ASWeapon::ASWeapon()
{
	MeshComp = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("MeshComp"));
	SetRootComponent(MeshComp);

	MuzzleSocketName = "MuzzleSocket";
	TracerTargetName = "Target";
}

void ASWeapon::Fire()
{
	// Trace the world from pawn eyes to crosshair location

	AActor* MyOwner = GetOwner();
	if (!MyOwner) { return; }

	FVector EyeLocation;
	FRotator EyeRotation;
	MyOwner->GetActorEyesViewPoint(EyeLocation, EyeRotation);

	FVector TraceEnd = EyeLocation + (EyeRotation.Vector() * 10000);

	FVector ShotDirection = EyeRotation.Vector();

	FCollisionQueryParams QueryParams;
	QueryParams.AddIgnoredActor(MyOwner);
	QueryParams.AddIgnoredActor(this);
	QueryParams.bTraceComplex = true;

	// Particle "Target" parameter
	FVector TracerEndPoint = TraceEnd;

	FHitResult Hit;
	if (GetWorld()->LineTraceSingleByChannel(
		Hit,
		EyeLocation,
		TraceEnd,
		ECC_Visibility,
		QueryParams
	))
	{
		// Blocking hit! Process damage

		AActor* HitActor = Hit.GetActor();

		UGameplayStatics::ApplyPointDamage(
			HitActor,
			20.f,
			ShotDirection,
			Hit,
			MyOwner->GetInstigatorController(),
			this,
			DamageType
		);

		if (ImpactEffect)
		{
			UGameplayStatics::SpawnEmitterAtLocation(
				GetWorld(),
				ImpactEffect,
				Hit.ImpactPoint,
				Hit.ImpactNormal.Rotation()
			);
		}
		TracerEndPoint = Hit.ImpactPoint;
	}

	if (DebugWeaponDrawing > 0)
	{
		DrawDebugLine(
			GetWorld(),
			EyeLocation,
			TraceEnd,
			FColor::Red,
			false,
			1.f,
			0,
			1.f
		);
	}
	
	PlayFireEffects(TracerEndPoint);
}

void ASWeapon::PlayFireEffects(FVector TracerEndPoint)
{
	if (MuzzleEffect)
	{
		UGameplayStatics::SpawnEmitterAttached(
			MuzzleEffect,
			MeshComp,
			MuzzleSocketName
		);
	}

	if (TracerEffect)
	{
		FVector MuzzleLocation = MeshComp->GetSocketLocation(MuzzleSocketName);
		UParticleSystemComponent* TracerComp = UGameplayStatics::SpawnEmitterAtLocation(
			GetWorld(),
			TracerEffect,
			MuzzleLocation
		);

		if (TracerComp)
		{
			TracerComp->SetVectorParameter(TracerTargetName, TracerEndPoint);
		}
	}
}

