// Fill out your copyright notice in the Description page of Project Settings.


#include "PTWMainMenu.h"
#include "Components/Button.h"
#include "PTWServerBrowser.h"
#include "Kismet/KismetSystemLibrary.h"
#include "UI/PTWUISubsystem.h"
#include "Components/Border.h"
#include "Components/CanvasPanelSlot.h"
#include "CoreFramework/MainMenu/PTWMainMenuPlayerController.h"
#include "System/PTWGameLiftClientSubsystem.h"

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

	if (MaskingBorder)
	{
		UCanvasPanelSlot* BorderSlot = Cast<UCanvasPanelSlot>(MaskingBorder->Slot);
		if (BorderSlot)
		{
			BorderSlot->SetSize(FVector2D(BorderSlot->GetSize().X, MaxHeight));
		}
	}

	if (ULocalPlayer* LP = GetOwningLocalPlayer())
	{
		if (UPTWUISubsystem* UISubsystem = LP->GetSubsystem<UPTWUISubsystem>())
		{
			UISubsystem->SetDefaultInputPolicy(EUIInputPolicy::UIOnly);
		}
	}
	
	SetIsEnabled(true);
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

void UPTWMainMenu::NativeTick(const FGeometry& MyGeometry, float InDeltaTime)
{
	Super::NativeTick(MyGeometry, InDeltaTime);

	if (!MaskingBorder) return;

	UCanvasPanelSlot* BorderSlot = Cast<UCanvasPanelSlot>(MaskingBorder->Slot);
	if (BorderSlot)
	{
		FVector2D CurrentSize = BorderSlot->GetSize();

		// 현재 높이에서 목표 높이로 부드럽게 보간
		float NewHeight = FMath::FInterpTo(CurrentSize.Y, TargetHeight, InDeltaTime, SlideSpeed);

		BorderSlot->SetSize(FVector2D(CurrentSize.X, NewHeight));
	}
}

void UPTWMainMenu::OnClickedPlayButton()
{
#if WITH_EDITOR
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
#else
	if (UPTWGameLiftClientSubsystem* GameLiftClientSubsystem = UPTWGameLiftClientSubsystem::Get(this))
	{
		GameLiftClientSubsystem->SearchQuickSession();
		SetIsEnabled(false);
	}
#endif
	
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

void UPTWMainMenu::ToggleMainMenu(bool bIsMenuOpen)
{
	if (bIsMenuOpen)
	{
		TargetHeight = MaxHeight;
	}
	else
	{
		TargetHeight = MinHeight;
	}
}
