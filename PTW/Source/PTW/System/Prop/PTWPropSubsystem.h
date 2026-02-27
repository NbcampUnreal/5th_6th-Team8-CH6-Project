// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "PTWPropSubsystem.generated.h"

UCLASS()
class PTW_API UPTWPropSubsystem : public UWorldSubsystem
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable)
	void RegisterByActorTag(FName GroupTag);

	UFUNCTION(BlueprintCallable)
	void SetGroupEnabled(FName GroupTag, bool bEnabled);
	
	UFUNCTION(BlueprintCallable)
	void RandomizeByActorTag(FName GroupTag, float EnableChance);

private:
	TMap<FName, TArray<TWeakObjectPtr<AActor>>> GroupToActors;

	void ApplyActorEnabled(AActor* Actor, bool bEnabled);
};
