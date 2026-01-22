// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "PTWMainMenu.generated.h"

class UCanvas;
class UButton;
class UWidgetSwitcher;
class UPTWLobbyBrowser;
/**
 * 
 */



UCLASS()
class PTW_API UPTWMainMenu : public UUserWidget
{
	GENERATED_BODY()
public:
	void OpenLobbyBrowser();
	
protected:
	virtual void NativeConstruct() override;
	virtual void NativeDestruct() override;
	
	UFUNCTION()
	void OnClickedPlayButton();
	UFUNCTION()
	void ReturnToMainMenu();
	
protected:
	UPROPERTY(meta=(BindWidget))
	TObjectPtr<UButton> PlayButton;
	
	UPROPERTY(meta=(BindWidget))
	TObjectPtr<UButton> OptionsButton;

	UPROPERTY(meta=(BindWidget))
	TObjectPtr<UButton> ExitButton;
	
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UWidgetSwitcher> MenuSwitcher;
	
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UWidget> MainMenuCanvas;
	
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UPTWLobbyBrowser> LobbyBrowser;
};
