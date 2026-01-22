// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AbilitySystemComponent.h"
#include "PTWAbilitySystemComponent.generated.h"


UCLASS()
class PTW_API UPTWAbilitySystemComponent : public UAbilitySystemComponent
{
	GENERATED_BODY()
	
public:
	void AbilityInputTagPressed(const FGameplayTag& InputTag);
	void AbilityInputTagReleased(const FGameplayTag& InputTag);

protected:

};
