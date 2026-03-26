// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "PTWMainMenu.generated.h"

class UCanvas;
class UButton;
class UWidgetSwitcher;
class UPTWServerBrowser;

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
	
protected:
	virtual void NativeConstruct() override;
	virtual void NativeDestruct() override;
	
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
};
