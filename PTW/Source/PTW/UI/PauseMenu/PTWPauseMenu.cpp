// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/PauseMenu/PTWPauseMenu.h"
#include "Components/Button.h"
#include "Engine/LocalPlayer.h"
#include "Kismet/KismetSystemLibrary.h"
#include "UI/PTWUISubsystem.h"
#include "GameFramework/PlayerController.h"
#include "System/PTWSessionSubsystem.h"

void UPTWPauseMenu::NativeConstruct()
{
	Super::NativeConstruct();

	if (Btn_Resume)
	{
		Btn_Resume->OnClicked.AddDynamic(this, &UPTWPauseMenu::OnClickedResume);
	}

	if (Btn_Options)
	{
		Btn_Options->OnClicked.AddDynamic(this, &UPTWPauseMenu::OnClickedOptions);
	}

	if (Btn_LeaveGame)
	{
		Btn_LeaveGame->OnClicked.AddDynamic(this, &UPTWPauseMenu::OnClickedLeaveGame);
	}

	if (Btn_QuitGame)
	{
		Btn_QuitGame->OnClicked.AddDynamic(this, &UPTWPauseMenu::OnClickedQuitGame);
	}
}

void UPTWPauseMenu::OnClickedResume()
{
	if (ULocalPlayer* LP = GetOwningLocalPlayer())
	{
		if (UPTWUISubsystem* UISubsystem = LP->GetSubsystem<UPTWUISubsystem>())
		{
			UISubsystem->PopWidget();
		}
	}
}

void UPTWPauseMenu::OnClickedOptions()
{
	if (ULocalPlayer* LP = GetOwningLocalPlayer())
	{
		if (UPTWUISubsystem* UISubsystem = LP->GetSubsystem<UPTWUISubsystem>())
		{
			// 나중에 연결
			// UISubsystem->PushWidget(OptionsMenuClass);
		}
	}
}

void UPTWPauseMenu::OnClickedLeaveGame()
{
	if (UGameInstance* GI = GetGameInstance())
	{
		if (UPTWSessionSubsystem* SessionSubsystem =
			GI->GetSubsystem<UPTWSessionSubsystem>())
		{
			//SessionSubsystem-> 세션 나가기();
		}
	}
}

void UPTWPauseMenu::OnClickedQuitGame()
{
	UKismetSystemLibrary::QuitGame(
		this,
		GetOwningPlayer(),
		EQuitPreference::Quit,
		false
	);
}
