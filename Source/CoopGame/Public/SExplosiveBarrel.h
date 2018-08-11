// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "SExplosiveBarrel.generated.h"

class USHealthComponent;
class UParticleSystem;
class UStaticMeshComponent;
class UMaterial;
class URadialForceComponent;

UCLASS()
class COOPGAME_API ASExplosiveBarrel : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	ASExplosiveBarrel();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	UFUNCTION()
	void OnHealthChanged(USHealthComponent* OwningHealthComp, float Health, float HealthDelta, const UDamageType* DamageType, AController* InstigatedBy, AActor* DamageCauser);

	UFUNCTION()
	void OnRep_bExploded();

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	USHealthComponent* HealthComp;
	
	UPROPERTY(ReplicatedUsing=OnRep_bExploded, BlueprintReadOnly, Category = "Explosion")
	bool bExploded;

	UPROPERTY(EditDefaultsOnly, Category = "Explosion")
	UParticleSystem* ExplosionEffect;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Components")
	UStaticMeshComponent* MeshComp;

	UPROPERTY(EditDefaultsOnly, Category = "Explosion")
	UMaterial* DefaultMaterial;

	UPROPERTY(EditDefaultsOnly, Category = "Explosion")
	UMaterial* ExplodedMaterial;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Explosion")
	URadialForceComponent* RadialForceComp;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	
};
