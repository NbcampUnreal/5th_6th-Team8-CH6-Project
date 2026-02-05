// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataTable.h"
#include "Inventory/PTWItemDefinition.h"
#include "PTWItemSpawnData.generated.h"

/**
 * 아이템 스폰 정보
 */
USTRUCT(BlueprintType)
struct FPTWItemSpawnRow : public FTableRowBase
{
	GENERATED_BODY()

public:
	// 아이템 정의 데이터 에셋
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	TSoftObjectPtr<UPTWItemDefinition> ItemDefinition;

	// 개발자용 메모
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FText DevComment;
};
