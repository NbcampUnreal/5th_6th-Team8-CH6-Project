// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameState.h"
#include "PTWGameState.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnRemainTimeChanged, int32, RemainTime);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnTimerFinished);
/**
 * 
 */
UCLASS()
class PTW_API APTWGameState : public AGameState
{
	GENERATED_BODY()

public:
	APTWGameState();

	void DecreaseTimer();
	
	UPROPERTY(BlueprintAssignable)
	FOnRemainTimeChanged OnRemainTimeChanged;

	FOnTimerFinished OnTimerFinished;
	
	UPROPERTY(VisibleAnywhere, ReplicatedUsing = OnRep_RemainTime, Category = "GameFlow");
	int32 RemainTime;

	UFUNCTION()
	void OnRep_RemainTime();

	
protected:
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	
	
};
