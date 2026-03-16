// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "PTWAbilityBoxWidget.generated.h"

class UButton;
class UTextBlock;
class UImage;
/**
 * 
 */
UCLASS()
class PTW_API UPTWAbilityBoxWidget : public UUserWidget
{
	GENERATED_BODY()

public:

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UButton> Button_AbilityButton;
	
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UImage> Image_AbilityIcon;
	
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> Text_AbilityName;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> Text_AbilityDescription;

	void InitAbilityBoxWidget();
};
