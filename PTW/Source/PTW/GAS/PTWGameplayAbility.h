// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Abilities/GameplayAbility.h"
#include "PTWGameplayAbility.generated.h"

class APTWBaseCharacter;
class APTWPlayerCharacter;
class APTWPlayerController;

UCLASS()
class PTW_API UPTWGameplayAbility : public UGameplayAbility
{
	GENERATED_BODY()

public:
	UPTWGameplayAbility();
	
	UPROPERTY(EditDefaultsOnly, Category = "Input")
	FGameplayTag StartupInputTag;

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Ability")
	APTWBaseCharacter* GetPTWCharacterFromActorInfo() const;
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Ability")
	APTWPlayerCharacter* GetPTWPlayerCharacterFromActorInfo() const;
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Ability")
	APTWPlayerController* GetPTWPlayerControllerFromActorInfo() const;
};
