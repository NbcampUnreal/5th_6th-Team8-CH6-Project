// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Components/Button.h"
#include "PTWButton.generated.h"

class USoundBase;
class UTextBlock;

UCLASS()
class PTW_API UPTWButton : public UUserWidget
{
	GENERATED_BODY()

protected:
	virtual void NativePreConstruct() override;
	virtual void NativeConstruct() override;
	virtual void NativeOnAddedToFocusPath(const FFocusEvent& InFocusEvent) override;
	virtual void NativeOnRemovedFromFocusPath(const FFocusEvent& InFocusEvent) override;
	virtual void NativeOnMouseEnter(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent) override;
	virtual void NativeOnMouseLeave(const FPointerEvent& InMouseEvent) override;

private:
	UFUNCTION()
	void HandleOnClicked();

	void ApplyFocusEffect(bool bIsFocused);

public:
	UPROPERTY(BlueprintAssignable, Category = "PTW|Event")
	FOnButtonClickedEvent OnClicked;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PTW|Button")
	FText ButtonText;

protected:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PTW|Sound")
	USoundBase* HoverSound;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PTW|Sound")
	USoundBase* ClickSound;


	UPROPERTY(meta = (BindWidget))
	UButton* InnerButton;
	UPROPERTY(meta = (BindWidget))
	UTextBlock* InnerText;
};
