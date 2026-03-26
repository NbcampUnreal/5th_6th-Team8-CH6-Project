// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/HUD.h"
#include "PTWHUD.generated.h"

class UPTWInGameHUD;
class UAbilitySystemComponent;

/**
 * 
 */
UCLASS()
class PTW_API APTWHUD : public AHUD
{
	GENERATED_BODY()
	
public:
	virtual void BeginPlay() override;

protected:
	void FindPlayerController();

private:
	FTimerHandle FindPlayerControllerTimerHandle;
};
