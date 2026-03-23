// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "PTWDraftCharge.generated.h"

class UImage;
class UTextBlock;
class UProgressBar;
/**
 * 
 */
UCLASS()
class PTW_API UPTWDraftCharge : public UUserWidget
{
	GENERATED_BODY()

public:
	UFUNCTION()
	void UpdateChargeTime(float RemainTime, float MaxTime);
	UFUNCTION()
	void UpdateChargeCount(int32 Count);

protected:
	void NativeConstruct() override;
	
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> Text_ChargeCount;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UImage> Image_Progress;

	UPROPERTY()
	TObjectPtr<UMaterialInstanceDynamic> ProgressMaterial;
	
	UPROPERTY(BlueprintReadOnly)
	float Percent;
};
