// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "ActiveGameplayEffectHandle.h"
#include "GameplayAbilitySpecHandle.h"
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
	
	virtual void ApplyChaosEvent(APTWGameState* GameState);
	virtual void ChaosEventEnd();
	
	
protected:
	UPROPERTY()
	TObjectPtr<UPTWChaosItemDefinition> Definition;

	int32 StackCount = 1;
	
	//* 적용된 어빌리티 제거를 위해 저장*/
	UPROPERTY()
	TMap<UAbilitySystemComponent*, FGameplayAbilitySpecHandle> ApplyAbilityHandles;
	
private:
	void ApplyChaosAbilityClass(APTWGameState* GameState);
	void ChaosAbilityEnd();
	
	
};
