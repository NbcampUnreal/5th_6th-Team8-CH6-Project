// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Abilities/GameplayAbility.h"
#include "PTWGA_BombExplode.generated.h"

UCLASS()
class PTW_API UPTWGA_BombExplode : public UGameplayAbility
{
	GENERATED_BODY()

public:
	UPTWGA_BombExplode();

protected:
	virtual void ActivateAbility(
		const FGameplayAbilitySpecHandle Handle,
		const FGameplayAbilityActorInfo* ActorInfo,
		const FGameplayAbilityActivationInfo ActivationInfo,
		const FGameplayEventData* TriggerEventData) override;

	UPROPERTY(EditDefaultsOnly, Category="Bomb|Explosion")
	float ExplosionRadius = 400.f;
};

