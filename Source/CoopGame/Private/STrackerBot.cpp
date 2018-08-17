// Fill out your copyright notice in the Description page of Project Settings.

#include "CoopGame/Public/STrackerBot.h"
#include <Components/StaticMeshComponent.h>
#include "NavigationSystem.h"
#include "NavigationPath.h"
#include <Kismet/GameplayStatics.h>
#include <GameFramework/Character.h>
#include "CoopGame/Public/SHealthComponent.h"
#include <Materials/MaterialInstanceDynamic.h>
#include "DrawDebugHelpers.h"
#include <Components/SphereComponent.h>
#include "Public/SCharacter.h"
#include "TimerManager.h"
#include "Sound/SoundCue.h"


// Sets default values
ASTrackerBot::ASTrackerBot()
{
	// Set this pawn to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	MeshComp = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("MeshComp"));
	MeshComp->SetCanEverAffectNavigation(false);
	MeshComp->SetSimulatePhysics(true);
	SetRootComponent(MeshComp);

	HealthComp = CreateDefaultSubobject<USHealthComponent>(TEXT("HealthComp"));
	HealthComp->OnHealthChanged.AddDynamic(this, &ASTrackerBot::HandleTakeDamage);

	SphereComp = CreateDefaultSubobject<USphereComponent>(TEXT("SphereComp"));
	SphereComp->SetSphereRadius(200.f);
	SphereComp->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	SphereComp->SetCollisionResponseToAllChannels(ECR_Ignore);
	SphereComp->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);
	SphereComp->SetCollisionResponseToChannel(ECC_WorldDynamic, ECR_Overlap);
	SphereComp->SetupAttachment(RootComponent);

	bUseVelocityChange = true;
	MovementForce = 1000.f;
	RequiredDistanceToTarget = 100.f;

	ExplosionDamage = 40.f;
	ExplosionRadius = 200.f;
	SelfDamageInterval = 0.2f;

	PowerLevel = 0;
	MaxPowerLevel = 10.f;
}

// Called when the game starts or when spawned
void ASTrackerBot::BeginPlay()
{
	Super::BeginPlay();

	if (Role == ROLE_Authority)
	{
		// Find initial move-to
		NexPathPoint = GetNextPathPoint();
	}

	GetWorldTimerManager().SetTimer(TimerHandle_PowerUp, this, &ASTrackerBot::PowerUp, 1.f, true, 0.f);
}

FVector ASTrackerBot::GetNextPathPoint()
{
	ACharacter* PlayerPawn = UGameplayStatics::GetPlayerCharacter(this, 0);
	if (!PlayerPawn)
	{
		return GetActorLocation();
	}

	UNavigationPath* NavPath = UNavigationSystemV1::FindPathToActorSynchronously(this, GetActorLocation(), PlayerPawn);
	if(!NavPath)
	{
		return GetActorLocation();
	}

	if (NavPath->PathPoints.Num() > 1)
	{
		// Return next point in the path
		return NavPath->PathPoints[1];
	}

	// Failed to find path
	return GetActorLocation();
}

void ASTrackerBot::HandleTakeDamage(USHealthComponent* OwningHealthComp, float Health, float HealthDelta, const UDamageType* DamageType, AController* InstigatedBy, AActor* DamageCauser)
{


	if (!MatInst)
	{
		MatInst = MeshComp->CreateAndSetMaterialInstanceDynamicFromMaterial(0, MeshComp->GetMaterial(0));
	}

	if(MatInst)
	{
		MatInst->SetScalarParameterValue("LastTimeDamageTaken", GetWorld()->TimeSeconds);

	}

	UE_LOG(LogTemp, Warning, TEXT("Current Health of %s: %f"), *GetName(), Health);

	// Explode on hitpoints == 0
	if (Health <= 0.f)
	{
		SelfDestruct();
	}
}

void ASTrackerBot::SelfDestruct()
{
	if (bExploded) { return; }

	bExploded = true;

	UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), ExplosionEffect, GetActorTransform());

	UGameplayStatics::PlaySoundAtLocation(this, ExplodeSound, GetActorLocation());

	MeshComp->SetVisibility(false, true);
	MeshComp->SetCollisionEnabled(ECollisionEnabled::NoCollision);


	if (Role == ROLE_Authority)
	{
		// Apply damage
		TArray<AActor*> IgnoredActors;
		IgnoredActors.Add(this);
		ExplosionDamage *= (PowerLevel + 1.f);
		UGameplayStatics::ApplyRadialDamage(this, ExplosionDamage, GetActorLocation(), ExplosionRadius, nullptr, IgnoredActors, this, GetInstigatorController(), true);
		DrawDebugSphere(GetWorld(), GetActorLocation(), ExplosionRadius, 32, FColor::Orange, false, 2.f, 0, 1.f);

		GetWorldTimerManager().ClearTimer(TimerHandle_SelfDamage);

		SetLifeSpan(2.f);
	}
}

void ASTrackerBot::DamageSelf()
{
	UGameplayStatics::ApplyDamage(this, 20, GetInstigatorController(), this, nullptr);
}

void ASTrackerBot::PowerUp()
{
	TArray<AActor*> OverlappingActors;
	SphereComp->GetOverlappingActors(OverlappingActors);

	int BotNum = 0;
	for (AActor* OverlapActor : OverlappingActors)
	{
		ASTrackerBot* Bot = Cast<ASTrackerBot>(OverlapActor);
		if (Bot)
		{
			++BotNum;
		}
	}

	PowerLevel = (float)BotNum;
	if (!MatInst)
	{
		MatInst = MeshComp->CreateAndSetMaterialInstanceDynamicFromMaterial(0, MeshComp->GetMaterial(0));
	}
	if (MatInst)
	{
		float Alpha = FMath::Clamp(PowerLevel / MaxPowerLevel, 0.f, 1.f);
		MatInst->SetScalarParameterValue("PowerLevelAlpha", Alpha);
	}
}

// Called every frame
void ASTrackerBot::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (Role == ROLE_Authority && !bExploded)
	{
		float DistanceToTarget = (GetActorLocation() - NexPathPoint).Size();

		if (DistanceToTarget <= RequiredDistanceToTarget)
		{
			NexPathPoint = GetNextPathPoint();
		}
		else
		{
			// Keep moving towards next target
			FVector ForceDirection = NexPathPoint - GetActorLocation();
			ForceDirection.Normalize();
			ForceDirection *= MovementForce;

			MeshComp->AddForce(ForceDirection, NAME_None, bUseVelocityChange);
		}
	}
}

void ASTrackerBot::NotifyActorBeginOverlap(AActor* OtherActor)
{
	Super::NotifyActorBeginOverlap(OtherActor);
	if (!bStartedSelfDestruction && !bExploded)
	{
		ASCharacter* PlayerPawn = Cast<ASCharacter>(OtherActor);
		if (PlayerPawn)
		{
			// Overlapped with a player!
			if (Role == ROLE_Authority)
			{
				//Start self destruction sequence
				GetWorldTimerManager().SetTimer(TimerHandle_SelfDamage, this, &ASTrackerBot::DamageSelf, SelfDamageInterval, true, 0.f);
			}

			bStartedSelfDestruction = true;

			UGameplayStatics::SpawnSoundAttached(SelfDestructSound, RootComponent);
		}
	}
}

