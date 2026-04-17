// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/PauseMenu/PTWPauseMenu.h"
#include "Components/Button.h"
#include "CoreFramework/Game/GameInstance/PTWGameInstance.h"
#include "Engine/LocalPlayer.h"
#include "Kismet/KismetSystemLibrary.h"
#include "UI/PTWUISubsystem.h"
#include "GameFramework/PlayerController.h"

void UPTWPauseMenu::NativeConstruct()
{
	Super::NativeConstruct();

	BindUIEvents();
}

void UPTWPauseMenu::NativeDestruct()
{
	UnbindUIEvents();

	Super::NativeDestruct();
}

void UPTWPauseMenu::BindUIEvents()
{
	UnbindUIEvents();

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

void UPTWPauseMenu::UnbindUIEvents()
{
	if (Btn_Resume)
	{
		Btn_Resume->OnClicked.RemoveDynamic(this, &UPTWPauseMenu::OnClickedResume);
	}

	if (Btn_Options)
	{
		Btn_Options->OnClicked.RemoveDynamic(this, &UPTWPauseMenu::OnClickedOptions);
	}

	if (Btn_LeaveGame)
	{
		Btn_LeaveGame->OnClicked.RemoveDynamic(this, &UPTWPauseMenu::OnClickedLeaveGame);
	}

	if (Btn_QuitGame)
	{
		Btn_QuitGame->OnClicked.RemoveDynamic(this, &UPTWPauseMenu::OnClickedQuitGame);
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
			// 현재 위젯(PauseMenu)을 스택에서 제거
			UISubsystem->PopWidget();

			// 설정된 OptionsMenuClass가 유효한지 확인 후 Push
			if (OptionsMenuClass)
			{
				UISubsystem->PushWidget(OptionsMenuClass, EUIInputPolicy::UIOnly);
			}
		}
	}
}

void UPTWPauseMenu::OnClickedLeaveGame()
{
	if (UPTWGameInstance* GI = GetGameInstance<UPTWGameInstance>())
	{
		GI->LeaveGameSession();
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
