// Fill out your copyright notice in the Description page of Project Settings.


#include "PTWMainMenu.h"
#include "Components/Button.h"
#include "Components/WidgetSwitcher.h"
#include "PTWServerBrowser.h"
#include "Kismet/KismetSystemLibrary.h"
#include "UI/PTWUISubsystem.h"

void UPTWMainMenu::OpenServerBrowser()
{
	if (IsValid(MenuSwitcher) && IsValid(ServerBrowser))
	{
		MenuSwitcher->SetActiveWidget(ServerBrowser);
	}
}

void UPTWMainMenu::NativeConstruct()
{
	Super::NativeConstruct();
	
	if (IsValid(PlayButton))
	{
		PlayButton->OnClicked.AddDynamic(this, &ThisClass::OnClickedPlayButton);
	}
	
	if (ServerBrowser)
	{
		ServerBrowser->OnServerBackAction.AddDynamic(this, &ThisClass::ReturnToMainMenu);
	}

	if (OptionsButton)
	{
		OptionsButton->OnClicked.AddDynamic(this, &ThisClass::OnClickedOptionsButton);
	}
	
	if (ExitButton)
	{
		ExitButton->OnClicked.AddDynamic(this, &ThisClass::OnClickedExitButton);
	}

	if (ULocalPlayer* LP = GetOwningLocalPlayer())
	{
		if (UPTWUISubsystem* UISubsystem = LP->GetSubsystem<UPTWUISubsystem>())
		{
			UISubsystem->SetDefaultInputPolicy(EUIInputPolicy::UIOnly);
		}
	}
}

void UPTWMainMenu::NativeDestruct()
{
	if (IsValid(PlayButton))
	{
		PlayButton->OnClicked.RemoveDynamic(this, &ThisClass::OnClickedPlayButton);
	}
	
	if (ServerBrowser)
	{
		ServerBrowser->OnServerBackAction.RemoveDynamic(this, &ThisClass::ReturnToMainMenu);
	}

	if (OptionsButton)
	{
		OptionsButton->OnClicked.RemoveDynamic(this, &ThisClass::OnClickedOptionsButton);
	}
	
	Super::NativeDestruct();
}

void UPTWMainMenu::OnClickedPlayButton()
{
	OpenServerBrowser();
}

void UPTWMainMenu::ReturnToMainMenu()
{
	UE_LOG(LogTemp, Warning, TEXT("Main Menu BACK"));
	if (IsValid(MenuSwitcher) && IsValid(MainMenuCanvas))
	{
		MenuSwitcher->SetActiveWidget(MainMenuCanvas);
	}
}

void UPTWMainMenu::OnClickedOptionsButton()
{
	if (ULocalPlayer* LP = GetOwningLocalPlayer())
	{
		if (UPTWUISubsystem* UISubsystem = LP->GetSubsystem<UPTWUISubsystem>())
		{
			// 설정된 OptionsMenuClass가 유효한지 확인 후 Push
			if (OptionsMenuClass)
			{
				UISubsystem->PushWidget(OptionsMenuClass, EUIInputPolicy::UIOnly);
			}
		}
	}
}

void UPTWMainMenu::OnClickedExitButton()
{
	UKismetSystemLibrary::QuitGame(this, GetOwningPlayer(), 
		EQuitPreference::Quit, false);
}
