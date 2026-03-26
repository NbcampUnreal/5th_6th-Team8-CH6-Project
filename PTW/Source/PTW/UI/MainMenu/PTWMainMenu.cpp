// Fill out your copyright notice in the Description page of Project Settings.


#include "PTWMainMenu.h"
#include "Components/Button.h"
#include "PTWServerBrowser.h"
#include "Kismet/KismetSystemLibrary.h"
#include "UI/PTWUISubsystem.h"

void UPTWMainMenu::NativeConstruct()
{
	Super::NativeConstruct();
	
	if (IsValid(PlayButton))
	{
		PlayButton->OnClicked.AddUniqueDynamic(this, &ThisClass::OnClickedPlayButton);
	}

	if (OptionsButton)
	{
		OptionsButton->OnClicked.AddUniqueDynamic(this, &ThisClass::OnClickedOptionsButton);
	}
	
	if (IsValid(ExitButton))
	{
		ExitButton->OnClicked.AddUniqueDynamic(this, &ThisClass::OnClickedExitButton);
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
	
	if (OptionsButton)
	{
		OptionsButton->OnClicked.RemoveDynamic(this, &ThisClass::OnClickedOptionsButton);
	}
	
	Super::NativeDestruct();
}

void UPTWMainMenu::OnClickedPlayButton()
{
	if (IsValid(ServerBrowserClass))
	{
		if (ULocalPlayer* LP = GetOwningLocalPlayer())
		{
			if (UPTWUISubsystem* UISubsystem = LP->GetSubsystem<UPTWUISubsystem>())
			{
				UISubsystem->HideSystemWidget(GetClass());
				UISubsystem->ShowSystemWidget(ServerBrowserClass);
			}
		}
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
