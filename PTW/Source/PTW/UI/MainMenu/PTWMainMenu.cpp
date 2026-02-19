// Fill out your copyright notice in the Description page of Project Settings.


#include "PTWMainMenu.h"
#include "Components/Button.h"
#include "Components/WidgetSwitcher.h"
#include "PTWServerBrowser.h"
#include "Kismet/KismetSystemLibrary.h"

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
	
	if (ExitButton)
	{
		ExitButton->OnClicked.AddDynamic(this, &ThisClass::OnClickedExitButton);
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

void UPTWMainMenu::OnClickedExitButton()
{
	UKismetSystemLibrary::QuitGame(this, GetOwningPlayer(), 
		EQuitPreference::Quit, false);
}
