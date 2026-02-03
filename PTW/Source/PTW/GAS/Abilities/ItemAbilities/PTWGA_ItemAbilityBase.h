// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GAS/PTWGameplayAbility.h"
#include "PTWGA_ItemAbilityBase.generated.h"

class UPTWInventoryComponent;
class UGameplayEffect;
class APTWPlayerCharacter;
/**
 * 
 */
UCLASS(Abstract)
class PTW_API UPTWGA_ItemAbilityBase : public UPTWGameplayAbility
{
	GENERATED_BODY()
public:
	virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData) override;
	virtual void ConsumeItem();
	virtual void InitializeVariable();
	virtual void ApplyItemEffect() PURE_VIRTUAL(UPTWGA_ItemAbilityBase::ApplyItemEffect, );
	
protected:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Ability Effect Class")
	TSubclassOf<UGameplayEffect> EffectClass;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Character")
	TObjectPtr<APTWPlayerCharacter> PlayerCharacter;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Character")
	TObjectPtr<UPTWInventoryComponent> InventoryComponent;
	
};
