// Fill out your copyright notice in the Description page of Project Settings.


#include "PTWMainMenuPlayerController.h"
#include "Blueprint/UserWidget.h"
#include "PTW/UI/MainMenu/PTWMainMenu.h"
#include "UI/PTWUISubsystem.h"
#include "UI/PTWPopupWidget.h"

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
		
		bShowMouseCursor = true;
		FInputModeUIOnly InputMode;
		SetInputMode(InputMode);
	}
}

void APTWMainMenuPlayerController::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	if (IsLocalPlayerController())
	{
		bShowMouseCursor = false;
		SetInputMode(FInputModeGameOnly());
	}
	
	Super::EndPlay(EndPlayReason);
}

void APTWMainMenuPlayerController::Popup(const FText& InText)
{
	if (!PopupWidgetClass) return;

	if (ULocalPlayer* LP = GetLocalPlayer())
	{
		UPTWUISubsystem* UISubsystem = LP->GetSubsystem<UPTWUISubsystem>();

		if (!UISubsystem) return;

		UISubsystem->PushWidget(PopupWidgetClass, EUIInputPolicy::UIOnly);

		UUserWidget* TopWidget = UISubsystem->GetTopWidget();

		if (UPTWPopupWidget* Popup = Cast<UPTWPopupWidget>(TopWidget))
		{
			Popup->SetMessage(InText);
		}
	}
}
