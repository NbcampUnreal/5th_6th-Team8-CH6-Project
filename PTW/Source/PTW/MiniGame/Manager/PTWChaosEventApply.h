// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "ActiveGameplayEffectHandle.h"
#include "GameplayTagContainer.h"
#include "MiniGame/Data/PTWChaosItemDefinition.h"
#include "UObject/Object.h"
#include "PTWChaosEventApply.generated.h"

class UAbilitySystemComponent;
class APTWGameState;

/**
 * 
 */
UCLASS()
class PTW_API UPTWChaosEventApply : public UObject
{
	GENERATED_BODY()

public:
	void InitDefinition(UPTWChaosItemDefinition* InDefinition);

	void SetStackCount(int32 Count);
	
	void ApplyChaosEvent(APTWGameState* GameState);
	void ChaosEventEnd();
	
	
protected:
	UPROPERTY()
	TObjectPtr<UPTWChaosItemDefinition> Definition;

	int32 StackCount = 1;
	
	//* 적용된 이펙트 제거를 위해 저장*/
	TMap<TObjectPtr<UAbilitySystemComponent>, FActiveGameplayEffectHandle> ApplyEffectHandles;
	
private:
	void ApplyChaosEffect(APTWGameState* GameState);
	void ChaosEffectEnd();
	
	
};
