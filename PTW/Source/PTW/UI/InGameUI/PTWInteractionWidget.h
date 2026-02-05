// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "PTWInteractionWidget.generated.h"

/**
 * 
 */
UCLASS()
class PTW_API UPTWInteractionWidget : public UUserWidget
{
	GENERATED_BODY()
	
protected:
	virtual void NativeConstruct() override;
	virtual void NativeDestruct() override;

	/* Found 델리게이트에서 호출 */
	UFUNCTION(BlueprintCallable)
	void ShowWithText(const FText& InText);

	/* Lost 델리게이트에서 호출 */
	UFUNCTION(BlueprintCallable)
	void Hide();

	UPROPERTY(meta = (BindWidget))
	class UTextBlock* ActionText;
};
