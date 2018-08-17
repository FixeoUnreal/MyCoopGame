// Fill out your copyright notice in the Description page of Project Settings.

#include "SPowerupActor.h"
#include "TimerManager.h"

// Sets default values
ASPowerupActor::ASPowerupActor()
{
	PowerupInterval = 1.f;
	TotalNrOfTicks = 10;
}

// Called when the game starts or when spawned
void ASPowerupActor::BeginPlay()
{
	Super::BeginPlay();

	
}

void ASPowerupActor::OnTickPowerUp()
{
	TicksProcessed++;

	OnPowerupTicked();

	if (TicksProcessed >= TotalNrOfTicks)
	{
		OnExpired();

		// Delete timer
		GetWorldTimerManager().ClearTimer(TimerHandle_PowerupTick);
	}
}

void ASPowerupActor::ActivatePowerup()
{
	OnActivated();

	if (PowerupInterval > 0.f)
	{
		GetWorldTimerManager().SetTimer(TimerHandle_PowerupTick, this, &ASPowerupActor::OnTickPowerUp, PowerupInterval, true);
	}
	else
	{
		OnTickPowerUp();
	}
}

