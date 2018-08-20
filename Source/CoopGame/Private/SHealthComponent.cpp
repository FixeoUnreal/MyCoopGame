// Fill out your copyright notice in the Description page of Project Settings.

#include "CoopGame/Public/SHealthComponent.h"
#include <GameFramework/Actor.h>
#include <UnrealNetwork.h>
#include "SGameMode.h"
#include "Engine/World.h"


// Sets default values for this component's properties
USHealthComponent::USHealthComponent()
{
	DefaultHealth = 100.f;
	bIsDead = false;

	SetIsReplicated(true);
}


// Called when the game starts
void USHealthComponent::BeginPlay()
{
	Super::BeginPlay();

	// Only hook if we are server
	if (GetOwnerRole() == ROLE_Authority)
	{
		AActor* MyOwner = GetOwner();
		if (MyOwner)
		{
			MyOwner->OnTakeAnyDamage.AddDynamic(this, &USHealthComponent::HandleTakeAnyDamage);
		}
	}
	

	Health = DefaultHealth;
}

void USHealthComponent::OnRep_Health(float OldHealth)
{
	float Damage = Health - OldHealth;

	OnHealthChanged.Broadcast(this, Health, Damage, nullptr, nullptr, nullptr);
}

void USHealthComponent::HandleTakeAnyDamage(AActor* DamagedActor, float Damage, const class UDamageType* DamageType, class AController* InstigatedBy, AActor* DamageCauser)
{
	if(Damage <= 0.f || bIsDead){ return; }

	// Update health clamped
	Health = FMath::Clamp(Health - Damage, 0.f, DefaultHealth);

	UE_LOG(LogTemp, Warning, TEXT("Heath changed: %f"), Health);
	bIsDead = Health <= 0.f;
	OnHealthChanged.Broadcast(this, Health, Damage, DamageType, InstigatedBy, DamageCauser);

	if (bIsDead)
	{
		ASGameMode* GM = Cast<ASGameMode>(GetWorld()->GetAuthGameMode());
		if (GM)
		{
			GM->OnActorKilled.Broadcast(GetOwner(), DamageCauser, InstigatedBy);
		}
	}
	
}

void USHealthComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(USHealthComponent, Health);
}

void USHealthComponent::Heal(float HealAmount)
{
	if(HealAmount <= 0.f || Health <= 0.f){ return; }

	Health = FMath::Clamp(Health + HealAmount, 0.f, DefaultHealth);

	UE_LOG(LogTemp, Warning, TEXT("Heath changed: %f (+%f)"), Health, HealAmount);
	OnHealthChanged.Broadcast(this, Health, -HealAmount, nullptr, nullptr, nullptr);
}

float USHealthComponent::GetHealth() const
{
	return Health;
}

