// Fill out your copyright notice in the Description page of Project Settings.


#include "PTWMainMenuPlayerController.h"
#include "Blueprint/UserWidget.h"
#include "PTW/UI/MainMenu/PTWMainMenu.h"
#include "UI/PTWUISubsystem.h"
#include "UI/PTWPopupWidget.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "CoreFramework/PTWGameUserSettings.h"

APTWMainMenuPlayerController::APTWMainMenuPlayerController()
{
	if (!IsValid(MainMenuClass))
	{
		MainMenuClass = MainMenuClass.Get();
	}
}

void APTWMainMenuPlayerController::SetupInputComponent()
{
	Super::SetupInputComponent();

	if (UEnhancedInputComponent* EIC = Cast<UEnhancedInputComponent>(InputComponent))
	{
		// (Tab)
		EIC->BindAction(
			Toggle,
			ETriggerEvent::Started,
			this,
			&APTWMainMenuPlayerController::ToggleMenu
		);
	}

	if (UPTWGameUserSettings* Settings = Cast<UPTWGameUserSettings>(UGameUserSettings::GetGameUserSettings()))
	{
		CurrentMouseSensitivity = Settings->MouseSensitivity;
	}

	UE_LOG(LogTemp, Warning, TEXT("[APTWMainMenuPlayerController] SetupInputComponent"));

}

void APTWMainMenuPlayerController::ApplyMouseSensitivity(float NewValue)
{
	CurrentMouseSensitivity = NewValue;
}

void APTWMainMenuPlayerController::BeginPlay()
{
	Super::BeginPlay();
	
	if (ULocalPlayer* LP = GetLocalPlayer())
	{
		if (UEnhancedInputLocalPlayerSubsystem* Subsystem =
			LP->GetSubsystem<UEnhancedInputLocalPlayerSubsystem>())
		{
			Subsystem->AddMappingContext(DefaultMappingContext, 0);
		}
	}

	if (!IsRunningDedicatedServer())
	{
		if (ULocalPlayer* LP = GetLocalPlayer())
		{
			if (UPTWUISubsystem* UISubsystem = LP->GetSubsystem<UPTWUISubsystem>())
			{
				UISubsystem->ShowSystemWidget(MainMenuClass);
				UISubsystem->SetDefaultInputPolicy(EUIInputPolicy::GameAndUI);

				UUserWidget* Menu = UISubsystem->GetOrCreateWidget(MainMenuClass);
				MenuWidget = Cast<UPTWMainMenu>(Menu);
			}
		}
		bShowMouseCursor = true;
		FInputModeUIOnly InputMode;
		SetInputMode(InputMode);
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
		FInputModeGameOnly InputModeData;
		SetInputMode(InputModeData);
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

void APTWMainMenuPlayerController::ToggleMenu()
{
	if (!MenuWidget) return;

	bIsMenuOpen = !bIsMenuOpen;

	MenuWidget->ToggleMainMenu(bIsMenuOpen);

	if (ULocalPlayer* LP = GetLocalPlayer())
	{
		UPTWUISubsystem* UISubsystem = LP->GetSubsystem<UPTWUISubsystem>();

		if (!UISubsystem) return;

		if (bIsMenuOpen)
		{
			UISubsystem->ApplyInputPolicy(EUIInputPolicy::GameAndUI);
		}
		else
		{
			UISubsystem->ApplyInputPolicy(EUIInputPolicy::GameOnly);
		}
	}
}
