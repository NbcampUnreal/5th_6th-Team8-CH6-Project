// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "PTWCountDownWidget.generated.h"

class UTextBlock;
class UWidgetAnimation;
/**
 * 
 */
UCLASS()
class PTW_API UPTWCountDownWidget : public UUserWidget
{
	GENERATED_BODY()
	
public:
	void UpdateCountDown(int32 Count);
	
public:
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> CountDownTextBlock;
	
	UPROPERTY(Transient, meta = (BindWidgetAnim))
	TObjectPtr<UWidgetAnimation> CountdownAnim;
};
