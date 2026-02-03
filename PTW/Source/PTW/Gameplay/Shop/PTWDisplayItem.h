// Fill out your copyright notice in the Description page of Project Settings.

// Source/PTW/Gameplay/Shop/PTWDisplayItem.h

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "PTWDisplayItem.generated.h"

class UWidgetComponent;
class APTWShopNPC;

UCLASS()
class PTW_API APTWDisplayItem : public AActor
{
	GENERATED_BODY()

public:
	APTWDisplayItem();

	void InitDisplay(FName NewItemID);
	void SetParentShop(APTWShopNPC* Shop) { ParentShop = Shop; }

	/* 상호작용 시 호출(구매 시도) */
	UFUNCTION(BlueprintCallable)
	void TryPurchase(APlayerController* Player);

protected:
	UPROPERTY(VisibleAnywhere)
	TObjectPtr<UStaticMeshComponent> ItemMesh;

	UPROPERTY(VisibleAnywhere)
	TObjectPtr<UWidgetComponent> InfoWidget;

	FName ItemID;

	UPROPERTY()
	TObjectPtr<APTWShopNPC> ParentShop;
};
