// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "ActiveGameplayEffectHandle.h"
#include "GameplayTagContainer.h"
#include "UObject/Object.h"
#include "PTWChaosEventApply.generated.h"

class UAbilitySystemComponent;
class APTWGameState;
class UPTWChaosItemDefinition;
/**
 * 
 */
UCLASS()
class PTW_API UPTWChaosEventApply : public UObject
{
	GENERATED_BODY()

public:
	void ApplyChaosEffect(APTWGameState* GameState);
	
	void ChaosEventApply(APTWGameState* GameState);
	
	void ChaosEventEnd();

	void InitDefinition(UPTWChaosItemDefinition* InDefinition);
	void InitHandles();
	
protected:
	UPROPERTY()
	TObjectPtr<UPTWChaosItemDefinition> Definition;
	
private:

	void Test();
	
	// 태그에 맞는 함수 저장
	TMap<FGameplayTag, TFunction<void()>> ChaosHandle;
	
	//* 적용된 이펙트 제거를 위해 저장*/
	TMap<TObjectPtr<UAbilitySystemComponent>, FActiveGameplayEffectHandle> ApplyEffectHandles;
	
};
