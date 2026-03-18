// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "GameplayTagContainer.h"
#include "PTWAPIData.generated.h"

UCLASS()
class PTW_API UPTWAPIData : public UDataAsset
{
	GENERATED_BODY()
	
public:
	FString GetAPIEndPoint(const FGameplayTag& APIEndPoint);
protected:
	UPROPERTY(EditDefaultsOnly)
	FString Name;

	UPROPERTY(EditDefaultsOnly)
	FString InvokeURL;

	UPROPERTY(EditDefaultsOnly)
	FString Stage;

	UPROPERTY(EditDefaultsOnly)
	TMap<FGameplayTag, FString> Resources;
};
