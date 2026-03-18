// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "PTWPopupWidget.generated.h"

class UTextBlock;
class UButton;

/**
 * 
 */
UCLASS()
class PTW_API UPTWPopupWidget : public UUserWidget
{
	GENERATED_BODY()
	
public:
	void SetMessage(const FText& InText);

protected:
	UPROPERTY(meta = (BindWidget))
	UTextBlock* MessageText;

	UPROPERTY(meta = (BindWidget))
	UButton* OkButton;

	virtual void NativeConstruct() override;

	UFUNCTION()
	void OnClickedOK();
};
