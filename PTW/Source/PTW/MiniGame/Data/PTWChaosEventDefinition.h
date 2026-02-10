// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "PTWChaosEventDefinition.generated.h"

/**
 * 
 */

class UGameplayEffect;

UCLASS(BlueprintType)
class PTW_API UPTWChaosEventDefinition : public UPrimaryDataAsset
{
	GENERATED_BODY()

public:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Event")
	FName EventId;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="UI")
	FText DisplayName;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="UI")
	FText Description;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="UI")
	TObjectPtr<UTexture2D> Icon;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="GAS")
	TSubclassOf<UGameplayEffect> GameplayEffectClass;

	//* 지속 시간이 설정되어 있으면 카오스 이벤트 지속 시간 보다 우선적으로 적용 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Event")
	float DurationSeconds = 10.f;
	
};
