// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Object.h"
#include "PTWItemInstance.generated.h"

class UPTWItemDefinition;
/**
 * 
 */
UCLASS()
class PTW_API UPTWItemInstance : public UObject
{
	GENERATED_BODY()
	
public:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	TObjectPtr<UPTWItemDefinition> ItemDef;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	int32 CurrentAmmo;
};
