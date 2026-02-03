// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "PTWRoundEvent.generated.h"

UENUM(BlueprintType)
enum class EPTWRoundEventType : uint8
{
	Normal  UMETA(DisplayName = "평범한 하루"),
	Depression	UMETA(DisplayName = "경제 대공황"),
	Inflation	UMETA(DisplayName = "인플레이션"),
	ThanosSnap	UMETA(DisplayName = "타노스 핑거"),
	BlackMarket	UMETA(DisplayName = "블랙 마켓"),
	ArmsEmbargo	UMETA(DisplayName = "무기 금수조치"),
	ClearanceSale	UMETA(DisplayName = "재고 떨이")
};

USTRUCT(BlueprintType)
struct FPTWRoundEventRow : public FTableRowBase
{
	GENERATED_BODY()
	
	UPROPERTY(EditAnywhere)
	EPTWRoundEventType EventType;

	UPROPERTY(EditAnywhere)
	FGameplayTag EventTag;

	UPROPERTY(EditAnywhere)
	FText EventName;

	UPROPERTY(EditAnywhere)
	FText EventDescription;
};
