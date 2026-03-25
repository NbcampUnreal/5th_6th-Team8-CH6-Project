// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Abilities/GameplayAbility.h"
#include "Engine/DataTable.h"
#include "PTWRandomItemBoxData.generated.h"

USTRUCT(BlueprintType)
struct FRandomItemBoxData : public FTableRowBase
{
	GENERATED_BODY()
	
public:
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	TSubclassOf<UGameplayAbility> RandomItemGA;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FText ActivateText;
};
