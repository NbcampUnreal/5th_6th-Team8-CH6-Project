// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "PTWRedLightChargeWidget.generated.h"

class UProgressBar;
class APTWRedLightCharacter;

UCLASS()
class PTW_API UPTWRedLightChargeWidget : public UUserWidget
{
	GENERATED_BODY()
	
protected:
	virtual void NativeConstruct() override;
	virtual void NativeTick(const FGeometry& MyGeometry, float InDeltaTime) override;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UProgressBar> ChargeProgressBar;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UProgressBar> BatteryProgressBar;

	TWeakObjectPtr<APTWRedLightCharacter> CachedRedLightChar;
};
