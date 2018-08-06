// Fill out your copyright notice in the Description page of Project Settings.

#include "SWeapon.h"
#include <Components/SkeletalMeshComponent.h>
#include <DrawDebugHelpers.h>
#include <Kismet/GameplayStatics.h>
#include "Particles/ParticleSystem.h"


// Sets default values
ASWeapon::ASWeapon()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	MeshComp = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("MeshComp"));
	SetRootComponent(MeshComp);

	MuzzleSocketName = "MuzzleSocket";
}

// Called when the game starts or when spawned
void ASWeapon::BeginPlay()
{
	Super::BeginPlay();
	
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
		
	}

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

	if (MuzzleEffect) 
	{ 
		UGameplayStatics::SpawnEmitterAttached(
			MuzzleEffect,
			MeshComp,
			MuzzleSocketName
		);
	}
	
}

// Called every frame
void ASWeapon::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

