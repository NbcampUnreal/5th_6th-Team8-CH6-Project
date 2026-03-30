// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "PTWMainMenu.generated.h"

class UCanvas;
class UButton;
class UWidgetSwitcher;
class UPTWServerBrowser;
class UBorder;

UCLASS()
class PTW_API UPTWMainMenu : public UUserWidget
{
	GENERATED_BODY()
public:
	UFUNCTION()
	void OnClickedPlayButton();
	UFUNCTION()
	void OnClickedOptionsButton();
	UFUNCTION()
	void OnClickedExitButton();

	void ToggleMainMenu(bool bIsMenuOpen);
	
protected:
	virtual void NativeConstruct() override;
	virtual void NativeDestruct() override;
	
	virtual void NativeTick(const FGeometry& MyGeometry, float InDeltaTime) override;

protected:
	UPROPERTY(meta=(BindWidget))
	TObjectPtr<UButton> PlayButton;
	
	UPROPERTY(meta=(BindWidget))
	TObjectPtr<UButton> OptionsButton;

	UPROPERTY(meta=(BindWidget))
	TObjectPtr<UButton> ExitButton;
	
	UPROPERTY(EditDefaultsOnly, Category = "UI")
	TSubclassOf<UPTWServerBrowser> ServerBrowserClass;

	UPROPERTY(EditDefaultsOnly, Category = "UI")
	TSubclassOf<UUserWidget> OptionsMenuClass;

	UPROPERTY(meta = (BindWidget))
	UBorder* MaskingBorder;

	// 설정값
	UPROPERTY(EditAnywhere, Category = "UI Animation")
	float SlideSpeed = 12.0f;
	UPROPERTY(EditAnywhere, Category = "UI Animation")
	float MaxHeight = 800.0f; 
	UPROPERTY(EditAnywhere, Category = "UI Animation")
	float MinHeight = 100.0f;

	float TargetHeight = MaxHeight;
};
