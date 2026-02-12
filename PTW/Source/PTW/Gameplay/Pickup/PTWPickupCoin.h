// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Gameplay/PTWPickupItemBase.h"
#include "PTWPickupCoin.generated.h"

/**
 * 
 */
UCLASS()
class PTW_API APTWPickupCoin : public APTWPickupItemBase
{
	GENERATED_BODY()
	
public:
	APTWPickupCoin();

protected:
	virtual void OnPickedUp(class APTWPlayerCharacter* Player) override;

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Coin")
	int32 GoldAmount = 100;
};
