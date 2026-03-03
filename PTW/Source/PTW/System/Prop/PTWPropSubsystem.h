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
	
	UFUNCTION(BlueprintCallable) //그룹 내 선택된 모든 액터들 랜덤 토글
	void ApplySeededRandomByActorTag(FName GroupTag, int32 Seed, float EnableChance);
	
	UFUNCTION(BlueprintCallable)// 그룹 자체를 랜덤 토글
	void ApplySeededRandomGroupEnabled(FName GroupTag, int32 Seed, float EnableChance);
	
	UFUNCTION(BlueprintCallable)
	void ApplyRoundPropSeed(int32 Seed);

private:
	TMap<FName, TArray<TWeakObjectPtr<AActor>>> GroupToActors;

	void ApplyActorEnabled(AActor* Actor, bool bEnabled);
};
