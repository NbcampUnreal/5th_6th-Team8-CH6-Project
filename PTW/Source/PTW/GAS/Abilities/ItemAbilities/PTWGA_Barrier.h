// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "PTWGA_ItemAbilityBase.h"
#include "PTWGA_Barrier.generated.h"
class ABarrierActor;

/**
 * 
 */
UCLASS()
class PTW_API UPTWGA_Barrier : public UPTWGA_ItemAbilityBase
{
	GENERATED_BODY()
	
public:
	virtual void ApplyItemEffect() override;
	
protected:
	FVector SpawnLocation();
	
protected:
	UPROPERTY(EditAnywhere)
	TSubclassOf<ABarrierActor> BarrierClass;
};
