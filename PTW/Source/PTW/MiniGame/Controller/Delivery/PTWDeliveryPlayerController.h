// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "CoreFramework/PTWPlayerController.h"
#include "PTWDeliveryPlayerController.generated.h"

/**
 * 
 */
UCLASS()
class PTW_API APTWDeliveryPlayerController : public APTWPlayerController
{
	GENERATED_BODY()
	
public:
	virtual void CreateUI() override;
	
protected:
	UPROPERTY(EditAnywhere)
	TSubclassOf<UUserWidget> BatteryWidgetClass;
	
};
