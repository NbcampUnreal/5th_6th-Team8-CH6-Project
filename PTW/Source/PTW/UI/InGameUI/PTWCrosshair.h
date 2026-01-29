// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "PTWCrosshair.generated.h"

/**
 * 
 */
UCLASS()
class PTW_API UPTWCrosshair : public UUserWidget
{
	GENERATED_BODY()
	
public:
	UFUNCTION()
	void SetCrosshairVisibility(bool bVisible);

protected:
	virtual void NativeConstruct() override;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional))
	TObjectPtr<class UImage> CrosshairImage;
};
