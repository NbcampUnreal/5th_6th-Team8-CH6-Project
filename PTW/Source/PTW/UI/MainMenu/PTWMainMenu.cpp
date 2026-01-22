// Fill out your copyright notice in the Description page of Project Settings.


#include "PTWMainMenu.h"
#include "Components/Button.h"
#include "Components/WidgetSwitcher.h"
#include "PTWLobbyBrowser.h"

void UPTWMainMenu::OpenLobbyBrowser()
{
	if (IsValid(MenuSwitcher) && IsValid(LobbyBrowser))
	{
		MenuSwitcher->SetActiveWidget(LobbyBrowser);
	}
}

void UPTWMainMenu::NativeConstruct()
{
	Super::NativeConstruct();
	
	if (IsValid(PlayButton))
	{
		PlayButton->OnClicked.AddDynamic(this, &ThisClass::OnClickedPlayButton);
	}
	
	if (LobbyBrowser)
	{
		LobbyBrowser->OnLobbyBackAction.AddDynamic(this, &ThisClass::ReturnToMainMenu);
	}
}

void UPTWMainMenu::NativeDestruct()
{
	if (IsValid(PlayButton))
	{
		PlayButton->OnClicked.RemoveDynamic(this, &ThisClass::OnClickedPlayButton);
	}
	
	if (LobbyBrowser)
	{
		LobbyBrowser->OnLobbyBackAction.RemoveDynamic(this, &ThisClass::ReturnToMainMenu);
	}
	
	Super::NativeDestruct();
}

void UPTWMainMenu::OnClickedPlayButton()
{
	OpenLobbyBrowser();
}

void UPTWMainMenu::ReturnToMainMenu()
{
	UE_LOG(LogTemp, Warning, TEXT("Main Menu BACK"));
	if (IsValid(MenuSwitcher) && IsValid(MainMenuCanvas))
	{
		MenuSwitcher->SetActiveWidget(MainMenuCanvas);
	}
}
