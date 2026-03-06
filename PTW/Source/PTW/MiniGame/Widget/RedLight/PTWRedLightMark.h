// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "PTWRedLightMark.generated.h"

/**
 * 
 */
UCLASS()
class PTW_API UPTWRedLightMark : public UUserWidget
{
	GENERATED_BODY()
	

public:
	UFUNCTION(BlueprintImplementableEvent, Category = "RedLight")
	void PlaySpottedAnimation();
};
