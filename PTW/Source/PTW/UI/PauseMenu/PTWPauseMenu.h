// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "PTWPauseMenu.generated.h"

class UButton;

/**
 * 
 */
UCLASS()
class PTW_API UPTWPauseMenu : public UUserWidget
{
	GENERATED_BODY()
	
protected:
	virtual void NativeConstruct() override;

	/** Buttons (BindWidget) */
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UButton> Btn_Resume;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UButton> Btn_Options;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UButton> Btn_LeaveGame;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UButton> Btn_QuitGame;

private:
	/** Button Callbacks */
	UFUNCTION()
	void OnClickedResume();

	UFUNCTION()
	void OnClickedOptions();

	UFUNCTION()
	void OnClickedLeaveGame();

	UFUNCTION()
	void OnClickedQuitGame();
};
