// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GAS/PTWGameplayAbility.h"
#include "PTWGA_Stealth.generated.h"

class UGameplayEffect;

/**
 * 
 */
UCLASS()
class PTW_API UPTWGA_Stealth : public UPTWGameplayAbility
{
	GENERATED_BODY()
protected:
	virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData) override;
	
	UFUNCTION()
	void OnStealthTimerFinished();
	
	UFUNCTION()
	void OnMovementDetected();
	
protected:
	UPROPERTY(EditAnywhere)
	TSubclassOf<UGameplayEffect> StealthEffectClass;
	
	FTimerHandle StealthTimerHandle;
};
