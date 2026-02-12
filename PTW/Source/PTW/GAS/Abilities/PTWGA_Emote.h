// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GAS/PTWGameplayAbility.h"
#include "PTWGA_Emote.generated.h"

/**
 * 
 */
UCLASS()
class PTW_API UPTWGA_Emote : public UPTWGameplayAbility
{
	GENERATED_BODY()
	
public:
	UPTWGA_Emote();

	virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData) override;

protected:
	UFUNCTION()
	void OnMontageCompleted();
	UFUNCTION()
	void OnMontageInterrupted();

protected:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Animation")
	TObjectPtr<UAnimMontage> EmoteMontage;
};
