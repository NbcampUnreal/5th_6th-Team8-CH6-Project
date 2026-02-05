// Fill out your copyright notice in the Description page of Project Settings.

// Source/PTW/Gameplay/Shop/PTWDisplayItem.h

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "CoreFramework/Interface/PTWInteractInterface.h"
#include "PTWDisplayItem.generated.h"

class UWidgetComponent;
class APTWShopNPC;

UCLASS()
class PTW_API APTWDisplayItem : public AActor, public IPTWInteractInterface
{
	GENERATED_BODY()

public:
	APTWDisplayItem();

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	void InitDisplay(FName NewItemID);
	void SetParentShop(APTWShopNPC* Shop) { ParentShop = Shop; }

	/* 상호작용 시 호출(구매 시도) */
	UFUNCTION(BlueprintCallable)
	void TryPurchase(APlayerController* Player);

	virtual void OnInteract_Implementation(APawn* InstigatorPawn) override;
	virtual FText GetInteractionKeyword_Implementation() override;
	virtual bool IsInteractable_Implementation() override;

protected:
	UFUNCTION()
	void OnRep_ItemID();

	void UpdateItemVisuals();

protected:
	UPROPERTY(VisibleAnywhere)
	TObjectPtr<UStaticMeshComponent> ItemMesh;

	UPROPERTY(VisibleAnywhere)
	TObjectPtr<UWidgetComponent> InfoWidget;

	UPROPERTY(ReplicatedUsing = OnRep_ItemID, EditAnywhere, BlueprintReadOnly, Category = "Shop")
	FName ItemID;

	UPROPERTY()
	TObjectPtr<APTWShopNPC> ParentShop;

	int32 CachedPrice = 0;
};
