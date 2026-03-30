// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "PTWMainMenuPlayerController.generated.h"

class UPTWUIControllerComponent;
class UPTWMainMenu;
class UPTWLobbyBrowser;
class UInputMappingContext;
class UInputAction;

UCLASS()
class PTW_API APTWMainMenuPlayerController : public APlayerController
{
	GENERATED_BODY()
	
public:
	APTWMainMenuPlayerController();

	virtual void SetupInputComponent() override;

	UFUNCTION(BlueprintCallable, Category = "UI")
	void Popup(const FText& InText);

	void ApplyMouseSensitivity(float NewValue);

protected:
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

	void ToggleMenu();

public:
	float CurrentMouseSensitivity = 1.0f;

protected:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "UI")
	TSubclassOf<UPTWMainMenu> MainMenuClass;

	UPROPERTY(EditDefaultsOnly, Category = "UI")
	TSubclassOf<UUserWidget> PopupWidgetClass;

	UPROPERTY()
	UPTWMainMenu* MenuWidget;

	UPROPERTY(EditDefaultsOnly, Category = "Input")
	UInputMappingContext* DefaultMappingContext;
	UPROPERTY(EditDefaultsOnly, Category = "Input")
	UInputAction* Toggle;

	bool bIsMenuOpen = true;
};
