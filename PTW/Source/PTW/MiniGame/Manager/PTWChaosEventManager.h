// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "PTWChaosEventManager.generated.h"


class APTWGameState;
class UPTWChaosEventDefinition;

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class PTW_API UPTWChaosEventManager : public UActorComponent
{
	GENERATED_BODY()

public:
	void ApplyChaosEvent();

	void InitGameState(APTWGameState* InGameState);
private:

	UPTWChaosEventDefinition* SelectRandomChaosEvent();
	
	UPROPERTY(EditDefaultsOnly)
	TArray<TObjectPtr<UPTWChaosEventDefinition>> ChaosEventDefinitions;

	UPROPERTY()
	TObjectPtr<APTWGameState> PTWGameState;
	
};
