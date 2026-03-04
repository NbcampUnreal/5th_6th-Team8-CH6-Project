// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "PTWPropData.generated.h"

USTRUCT(BlueprintType)
struct FPTWPropGroupRule
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FName GroupTag;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta=(ClampMin="0.0", ClampMax="1.0"))
	float EnableChance = 0.5f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	int32 SeedSalt = 0;
};

UCLASS(BlueprintType)
class PTW_API UPTWPropData : public UDataAsset
{
	GENERATED_BODY()

public:

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	TArray<FPTWPropGroupRule> Rules;
};
