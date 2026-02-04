// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameplayEffectTypes.h"
#include "PTWItemInstance.h"
#include "PTWPassiveItemInstance.generated.h"

/**
 * 
 */
UCLASS()
class PTW_API UPTWPassiveItemInstance : public UPTWItemInstance
{
	GENERATED_BODY()
	
public:
	void AddPassiveSpecHandles(const FActiveGameplayEffectHandle& SpecHandle);
	void RemovePassiveSpecHandles(const FActiveGameplayEffectHandle& SpecHandle);
	void RemoveAllPassiveSpecHandles();
	FORCEINLINE TArray<FActiveGameplayEffectHandle> GetPassiveSpecHandleArr() { return PassiveSpecHandles; }
	
	
protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Passive")
	TArray<FActiveGameplayEffectHandle> PassiveSpecHandles;
};
