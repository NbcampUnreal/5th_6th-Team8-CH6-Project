// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "Engine/DataAsset.h"
#include "PTWWeaponSoundTable.generated.h"

USTRUCT(BlueprintType)
struct FWeaponSoundData
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FGameplayTag WeaponTag;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	USoundCue* Sound;
};


UCLASS()
class PTW_API UPTWWeaponSoundTable : public UDataAsset
{
	GENERATED_BODY()
	
public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sounds")
	TArray<FWeaponSoundData> SoundTable;
	
	UFUNCTION(BlueprintCallable, Category = "Sounds")
	USoundCue* GetSoundForTag(FGameplayTag Container) const;
};
