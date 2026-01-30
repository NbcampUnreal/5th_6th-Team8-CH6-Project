// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "GameplayTagContainer.h"
#include "PTWCombatInterface.generated.h"

// This class does not need to be modified.
UINTERFACE(MinimalAPI)
class UPTWCombatInterface : public UInterface
{
	GENERATED_BODY()
};

/**
 * 
 */
class PTW_API IPTWCombatInterface
{
	GENERATED_BODY()
public:
	virtual float GetDamageMultiplier(const FName& BoneName) = 0;
	
	virtual void HandleHitReaction(const FGameplayTag& HitTag) = 0;
	
};
