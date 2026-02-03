// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Gameplay/Shop/PTWShopItemData.h"
#include "PTWShopSpot.generated.h"

class APTWShopNPC;
class UArrowComponent;

UCLASS()
class PTW_API APTWShopSpot : public AActor
{
	GENERATED_BODY()
	
public:
	APTWShopSpot();

protected:
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

public:
	/* 아이템 3개가 놓일 위치 반환(로컬 좌표) */
	const TArray<FTransform>& GetItemSpawnTransforms() const { return ItemSpawnTransforms; }

protected:
	UPROPERTY(VisibleAnywhere)
	TObjectPtr<UStaticMeshComponent> StandMesh;

	UPROPERTY(EditAnywhere, Category = "Config", meta = (MakeEditWidget = true))
	TArray<FTransform> ItemSpawnTransforms;

	UPROPERTY()
	TObjectPtr<UBillboardComponent> Sprite;
	UPROPERTY()
	TObjectPtr<UArrowComponent> Arrow;
};
