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
		MainMenuClass = MainMenuClass.Get();
	}
}

void APTWMainMenuPlayerController::BeginPlay()
{
	Super::BeginPlay();
	
	if (!IsRunningDedicatedServer())
	{
		if (ULocalPlayer* LP = GetLocalPlayer())
		{
			if (UPTWUISubsystem* UISubsystem = LP->GetSubsystem<UPTWUISubsystem>())
			{
				UISubsystem->ShowSystemWidget(MainMenuClass);
				UISubsystem->SetDefaultInputPolicy(EUIInputPolicy::UIOnly);
			}
		}
		bShowMouseCursor = true;
		// FInputModeUIOnly InputMode;
		// SetInputMode(InputMode);
	}
}

void APTWMainMenuPlayerController::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	if (!IsRunningDedicatedServer())
	{
		if (ULocalPlayer* LP = GetLocalPlayer())
		{
			if (UPTWUISubsystem* UISubsystem = LP->GetSubsystem<UPTWUISubsystem>())
			{
				UISubsystem->HideSystemWidget(MainMenuClass);
				UISubsystem->SetDefaultInputPolicy(EUIInputPolicy::GameOnly);
			}
		}
		bShowMouseCursor = false;
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
