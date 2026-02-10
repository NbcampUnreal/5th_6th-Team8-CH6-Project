// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "ActiveGameplayEffectHandle.h"
#include "UObject/Object.h"
#include "PTWChaosEventApply.generated.h"

class UAbilitySystemComponent;
class APTWGameState;
class UPTWChaosEventDefinition;
/**
 * 
 */
UCLASS()
class PTW_API UPTWChaosEventApply : public UObject
{
	GENERATED_BODY()

public:
	void ChaosEventApply(APTWGameState* GameState);
	void ChaosEventEnd();

	void InitDefinition(UPTWChaosEventDefinition* InDefinition);


protected:
	UPROPERTY()
	TObjectPtr<UPTWChaosEventDefinition> Definition;
	
private:
	//* 적용된 이펙트 제거를 위해 저장*/
	TMap<TObjectPtr<UAbilitySystemComponent>, FActiveGameplayEffectHandle> ApplyEffectHandles;
	
};
