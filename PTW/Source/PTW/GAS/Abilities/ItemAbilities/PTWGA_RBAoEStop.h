// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GAS/PTWGameplayAbility.h"
#include "PTWGA_RBAoEStop.generated.h"

/**
 * 
 */
UCLASS()
class PTW_API UPTWGA_RBAoEStop : public UPTWGameplayAbility
{
	GENERATED_BODY()
	
protected:
	virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData) override;
	void ApplyAoEEffect(TArray<FOverlapResult>& OverlapResults);
	void SendActivateMsg(APTWPlayerController* TargetController);
protected:
	UPROPERTY(EditAnywhere)
	float DetectRad;
	
	UPROPERTY(EditAnywhere)
	TSubclassOf<UGameplayEffect> StunEffect;
};
