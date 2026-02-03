// Fill out your copyright notice in the Description page of Project Settings.

// Source/PTW/Gameplay/Shop/PTWShopNPC.h

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Gameplay/Shop/PTWShopItemData.h"
#include "PTWShopNPC.generated.h"

class APTWDisplayItem;

UCLASS()
class PTW_API APTWShopNPC : public AActor
{
	GENERATED_BODY()

public:
	APTWShopNPC();

	/* Subsystem에서 호출.카테고리와 판매할 아이템 ID 목록을 받음 */
	void InitializeShop(EShopCategory InCategory, const TArray<FName>& InItemIDs, const TArray<FTransform>& DisplayLocs);

protected:
	virtual void BeginPlay() override;

	void CheckShopAvailability();

	UFUNCTION(BlueprintImplementableEvent)
	void UpdateVisualState(bool bIsOpen);

protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	TObjectPtr<USkeletalMeshComponent> NPCMesh;

	UPROPERTY(Replicated, VisibleInstanceOnly)
	EShopCategory ShopCategory;

	UPROPERTY()
	TArray<TObjectPtr<APTWDisplayItem>> DisplayItems;

	UPROPERTY(EditDefaultsOnly)
	TSubclassOf<APTWDisplayItem> DisplayItemClass;

	bool bIsLocallyOpen;
};
