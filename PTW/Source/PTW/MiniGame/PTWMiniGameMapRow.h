// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "PTWMiniGameMapRow.generated.h"
/**
 * 
 */

UENUM(BlueprintType)
enum class EMiniGameMapType : uint8
{
	Bomb	UMETA(DisplayName = "폭탄 돌리기"),
	Delivery UMETA(DisplayName = "배달 레이스"),
	GhostChase UMETA(DisplayName = "고스트 꼬리잡기"),
	CopsandRobbers	UMETA(DisplayName = "경찰과 도둑"),
	Abyss     UMETA(DisplayName = "어비스"),
	AbilityBattle     UMETA(DisplayName = "능력 배틀"),
	RedLight     UMETA(DisplayName = "무궁화 꽃이 피었습니다")
};

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
