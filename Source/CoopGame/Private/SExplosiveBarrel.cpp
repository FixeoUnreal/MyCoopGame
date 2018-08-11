// Fill out your copyright notice in the Description page of Project Settings.

#include "CoopGame/Public/SExplosiveBarrel.h"
#include "Public/SHealthComponent.h"
#include <Kismet/GameplayStatics.h>
#include <Components/StaticMeshComponent.h>
#include "Materials/Material.h"
#include <PhysicsEngine/RadialForceComponent.h>
#include <UnrealNetwork.h>

// Sets default values
ASExplosiveBarrel::ASExplosiveBarrel()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	MeshComp = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("MeshComp"));
	SetRootComponent(MeshComp);

	HealthComp = CreateDefaultSubobject<USHealthComponent>(TEXT("HealthComp"));

	RadialForceComp = CreateDefaultSubobject<URadialForceComponent>(TEXT("RadialForceComp"));
	RadialForceComp->SetupAttachment(RootComponent);

	SetReplicates(true);
	SetReplicateMovement(true);
}

// Called when the game starts or when spawned
void ASExplosiveBarrel::BeginPlay()
{
	Super::BeginPlay();
	
	HealthComp->OnHealthChanged.AddDynamic(this, &ASExplosiveBarrel::OnHealthChanged);

}

void ASExplosiveBarrel::OnHealthChanged(USHealthComponent* OwningHealthComp, float Health, float HealthDelta, const UDamageType* DamageType, AController* InstigatedBy, AActor* DamageCauser)
{
	if (Health <= 0.f && !bExploded)
	{
		bExploded = true;
		OnRep_bExploded();

		if (MeshComp && MeshComp->IsSimulatingPhysics())
		{
			FVector LaunchVelocity(0, 0, 300);
			MeshComp->AddImpulse(LaunchVelocity, NAME_None, true);
		}

		if (RadialForceComp)
		{
			RadialForceComp->FireImpulse();
			UE_LOG(LogTemp, Warning, TEXT("After fire impulse"));
		}

		
	}
}

void ASExplosiveBarrel::OnRep_bExploded()
{
	if (ExplosionEffect)
	{
		UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), ExplosionEffect, GetActorTransform());
	}

	if (ExplodedMaterial && MeshComp)
	{
		MeshComp->SetMaterial(0, ExplodedMaterial);
	}
}

// Called every frame
void ASExplosiveBarrel::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

void ASExplosiveBarrel::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ASExplosiveBarrel, bExploded);
}

