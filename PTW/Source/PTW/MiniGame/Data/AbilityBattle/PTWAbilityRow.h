// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "PTWAbilityRow.generated.h"

class UPTWAbilityDefinition;
/**
 * 
 */
USTRUCT(BlueprintType)
struct FPTWAbilityRow : public FTableRowBase
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere)
	int32 Tier;
	
	UPROPERTY(EditAnywhere)
	TSoftObjectPtr<UPTWAbilityDefinition> AbilityDefinition;
};
