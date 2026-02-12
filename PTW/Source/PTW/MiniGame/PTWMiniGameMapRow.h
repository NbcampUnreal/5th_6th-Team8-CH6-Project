// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "PTWMiniGameMapRow.generated.h"
/**
 * 
 */

USTRUCT(BlueprintType)
struct FPTWMiniGameMapRow : public FTableRowBase
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere)
	TSoftObjectPtr<UWorld> Map;

	UPROPERTY(EditAnywhere)
	FText DisplayName;

	UPROPERTY(EditAnywhere)
	FGameplayTag MiniGameTag;
	
	UPROPERTY(EditAnywhere)
	TSoftObjectPtr<UTexture2D> Thumbnail;

	UPROPERTY(EditAnywhere, meta = (ClampMin = "1"))
	int32 MinPlayers = 1;

	UPROPERTY(EditAnywhere, meta = (ClampMax = "16"))
	int32 MaxPlayers = 16;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FText MapDescription;
};
