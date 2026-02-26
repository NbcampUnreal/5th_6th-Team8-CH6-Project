// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "PTWChaosItemRow.generated.h"

class UPTWChaosItemDefinition;
/**
 * 
 */
USTRUCT(BlueprintType)
struct FPTWChaosItemRow : public FTableRowBase
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere)
	TSoftObjectPtr<UPTWChaosItemDefinition> ChaosItemDefinition;
};
