// Fill out your copyright notice in the Description page of Project Settings.


#include "PTWMainMenuPlayerController.h"
#include "Blueprint/UserWidget.h"
#include "PTW/UI/MainMenu/PTWMainMenu.h"
#include "PTW/UI/MainMenu/PTWLobbyBrowser.h"

APTWMainMenuPlayerController::APTWMainMenuPlayerController()
{
	if (!IsValid(MainMenuClass))
	{
		MainMenuClass = MainMenuInstance->StaticClass();
	}
}

void APTWMainMenuPlayerController::BeginPlay()
{
	Super::BeginPlay();
	
	if (IsLocalPlayerController())
	{
		MainMenuInstance = CreateWidget<UPTWMainMenu>(this, MainMenuClass);
		
		if (IsValid(MainMenuInstance))
		{
			MainMenuInstance->AddToViewport();
		}
	}
	
	bShowMouseCursor = true;
	FInputModeUIOnly InputMode;
	InputMode.SetWidgetToFocus(MainMenuInstance->TakeWidget());
	SetInputMode(InputMode);
}
