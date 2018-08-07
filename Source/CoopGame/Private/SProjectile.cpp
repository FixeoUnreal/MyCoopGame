// Fill out your copyright notice in the Description page of Project Settings.

#include "CoopGame/Public/SProjectile.h"
#include "Components/SphereComponent.h"
#include "GameFramework/ProjectileMovementComponent.h"
#include "TimerManager.h"
#include "Kismet/GameplayStatics.h"
#include "Components/StaticMeshComponent.h"
#include "Particles/ParticleSystem.h"
#include "GameFramework/MovementComponent.h"
#include "Engine/World.h"

// Sets default values
ASProjectile::ASProjectile()
{
	/*Scene = CreateDefaultSubobject<USceneComponent>(TEXT("Scene"));
	SetRootComponent(Scene);*/

	// Use a sphere as a simple collision representation
	CollisionComp = CreateDefaultSubobject<USphereComponent>(TEXT("SphereComp"));
	CollisionComp->InitSphereRadius(5.0f);
	CollisionComp->SetCollisionProfileName("Projectile");
	CollisionComp->OnComponentHit.AddDynamic(this, &ASProjectile::OnHit);	// set up a notification for when this component hits something blocking

																			// Players can't walk on it
	CollisionComp->SetWalkableSlopeOverride(FWalkableSlopeOverride(WalkableSlope_Unwalkable, 0.f));
	CollisionComp->CanCharacterStepUpOn = ECB_No;
	SetRootComponent(CollisionComp);

	MeshComp = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("MeshComp"));
	MeshComp->SetupAttachment(CollisionComp);

	// Use a ProjectileMovementComponent to govern this projectile's movement
	ProjectileMovement = CreateDefaultSubobject<UProjectileMovementComponent>(TEXT("ProjectileComp"));
	ProjectileMovement->UpdatedComponent = CollisionComp;
	ProjectileMovement->InitialSpeed = 900.f;
	ProjectileMovement->MaxSpeed = 900.f;
	ProjectileMovement->bRotationFollowsVelocity = true;
	ProjectileMovement->bShouldBounce = true;

	// Die after 3 seconds by default
	//InitialLifeSpan = 3.0f;

}

void ASProjectile::Detonate()
{
	bool bDamageCaused = UGameplayStatics::ApplyRadialDamage(
		this,
		1000.f,
		GetActorLocation(),
		200.f,
		DamageType,
		TArray<AActor*>(),
		this,
		GetInstigatorController(),
		true
	);


	if (ExplosionEffect)
	{
		UGameplayStatics::SpawnEmitterAtLocation(
			GetWorld(),
			ExplosionEffect,
			GetActorTransform()
		);
	}

	Destroy();
}

void ASProjectile::OnHit(UPrimitiveComponent* HitComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit)
{

	FTimerHandle TimerHandle;
	GetWorldTimerManager().SetTimer(TimerHandle, this, &ASProjectile::Detonate, 1.f);
}

