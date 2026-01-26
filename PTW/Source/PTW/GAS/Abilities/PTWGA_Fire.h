// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GAS/PTWGameplayAbility.h"
#include "PTWGA_Fire.generated.h"

/**
 * 
 */
UCLASS()
class PTW_API UPTWGA_Fire : public UPTWGameplayAbility
{
	GENERATED_BODY()
	
public:
	UPTWGA_Fire();
	
	virtual void InputPressed(const FGameplayAbilitySpecHandle Handle, 
		const FGameplayAbilityActorInfo* ActorInfo, 
		const FGameplayAbilityActivationInfo ActivationInfo) override;
	
	virtual void InputReleased(const FGameplayAbilitySpecHandle Handle, 
		const FGameplayAbilityActorInfo* ActorInfo, 
		const FGameplayAbilityActivationInfo ActivationInfo) override;
	
	virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle, 
		const FGameplayAbilityActorInfo* ActorInfo, 
		const FGameplayAbilityActivationInfo ActivationInfo, 
		const FGameplayEventData* TriggerEventData) override;
	
	void StartFire(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo,
							   const FGameplayAbilityActivationInfo ActivationInfo);
	void AutoFire(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo,
							   const FGameplayAbilityActivationInfo ActivationInfo);
	void StopFire();
	
protected:
	FTimerHandle AutoFireTimer;
	float FireRate = 0.15f;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Cue|Effect")
	TSubclassOf<UGameplayEffect> FireEffectClass;
	
};
