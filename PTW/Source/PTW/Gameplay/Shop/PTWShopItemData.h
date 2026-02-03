// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataTable.h"
#include "GameplayTagContainer.h"
#include "PTWShopItemData.generated.h"

class UPTWItemDefinition;
class UGameplayEffect;

UENUM(BlueprintType)
enum class EShopCategory : uint8
{
	None,
	Attack		UMETA(DisplayName = "Attack (Red)"),
	Defense		UMETA(DisplayName = "Defense (Blue)"),
	Utility		UMETA(DisplayName = "Utility (Green)"),
	Chaos		UMETA(DisplayName = "Chaos (Purple)"),
	Lobby		UMETA(DisplayName = "Lobby (Gold)")
};

UENUM(BlueprintType)
enum class EPurchaseType : uint8
{
	InventoryItem,		// 인벤토리에 아이템 추가
	LobbyAction			// 별도의 아이템 지급 없이 로직/UI만 실행
};

USTRUCT(BlueprintType)
struct FShopItemRow : public FTableRowBase
{
	GENERATED_BODY()

public:
	//표시될 아이템 이름
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Display")
	FText DisplayName;

	//아이템 설명
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Display")
	FText Description;

	//아이템 기본 가격
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Cost")
	int32 BasePrice;

	//상점 카테고리
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Type")
	EShopCategory Category;

	//등장을 금지시킬 미니게임 태그
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Type")
	FGameplayTagContainer BannedGameModes;

	//구매후 적용방식
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Type")
	EPurchaseType PurchaseType;

	// --- [데이터 페이로드 (PurchaseType에 따라 골라 씀)] ---

	// 1. [InventoryItem] 용도: 실제 인벤토리에 들어갈 아이템
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Payload", meta = (EditCondition = "PurchaseType == EPurchaseType::InventoryItem"))
	TSoftObjectPtr<UPTWItemDefinition> ItemDefinition;

	// 2. [LobbyAction] 용도: 로직 식별용 태그
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Payload", meta = (EditCondition = "PurchaseType == EPurchaseType::LobbyAction"))
	FGameplayTag ActionTag;

	// 3D 상점에 전시할 때 보여줄 메쉬
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Visual")
	TSoftObjectPtr<UStaticMesh> DisplayMesh;
};
