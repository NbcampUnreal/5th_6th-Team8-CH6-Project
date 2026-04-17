// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "PTWGameModeBaseComponent.generated.h"


struct FPTWMiniGameRule;
class APTWGameState;

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class PTW_API UPTWGameModeBaseComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	UPTWGameModeBaseComponent();

protected:
	virtual void BeginPlay() override;

protected:
	UPROPERTY()
	TObjectPtr<APTWGameState> GameState;

	const FPTWMiniGameRule* MiniGameRule;
};
