// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "PTWPropSubsystem.generated.h"

class UPTWPropData;

UCLASS()
class PTW_API UPTWPropSubsystem : public UWorldSubsystem
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable)
	void RegisterByActorTag(FName GroupTag);

	UFUNCTION(BlueprintCallable)
	void SetGroupEnabled(FName GroupTag, bool bEnabled);

	// 그룹 내 액터들을 랜덤 ON/OFF
	UFUNCTION(BlueprintCallable)
	void ApplySeededRandomByActorTag(FName GroupTag, int32 Seed, float EnableChance);

	// 그룹 전체 ON/OFF
	UFUNCTION(BlueprintCallable)
	void ApplySeededRandomGroupEnabled(FName GroupTag, int32 Seed, float EnableChance);
	
	UFUNCTION(BlueprintCallable)
	void ApplyPropDataSeeded(const UPTWPropData* Data, int32 Seed);

private:
	TMap<FName, TArray<TWeakObjectPtr<AActor>>> GroupToActors;

	void ApplyActorEnabled(AActor* Actor, bool bEnabled);
};
