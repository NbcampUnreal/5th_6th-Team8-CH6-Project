// Fill out your copyright notice in the Description page of Project Settings.


#include "PTWSpectatorHUD.h"
#include "Components/Border.h"
#include "Components/TextBlock.h"
#include "CoreFramework/PTWPlayerController.h"

void UPTWSpectatorHUD::SetSpectateTargetName(const FString& NewName)
{
	// SpectateTargetBorder->SetVisibility(ESlateVisibility::HitTestInvisible);
	SpectateTargetText->SetText(FText::FromString(NewName));
}

void UPTWSpectatorHUD::HandleNativeVisibilityChanged(ESlateVisibility InVisibility)
{
	APTWPlayerController* PC = GetOwningPlayer<APTWPlayerController>();
	if (!IsValid(PC)) return;

	switch (InVisibility)
	{
	case ESlateVisibility::Visible:
	case ESlateVisibility::HitTestInvisible:
	case ESlateVisibility::SelfHitTestInvisible:
		// SpectateTargetBorder->SetVisibility(ESlateVisibility::HitTestInvisible);
		break;
	
	case ESlateVisibility::Hidden:
	case ESlateVisibility::Collapsed:
		// SpectateTargetBorder->SetVisibility(ESlateVisibility::Collapsed);
	default:
		break;
	}
}

void UPTWSpectatorHUD::NativeConstruct()
{
	Super::NativeConstruct();
	
	// OnNativeVisibilityChanged.AddUObject(this, &ThisClass::HandleNativeVisibilityChanged);
	if (APTWPlayerController* PC = GetOwningPlayer<APTWPlayerController>())
	{
		PC->OnSpectateTargetChanged.AddUniqueDynamic(this, &ThisClass::SetSpectateTargetName);
	}
}

void UPTWSpectatorHUD::NativeDestruct()
{
	// OnNativeVisibilityChanged.RemoveAll(this);
	if (APTWPlayerController* PC = GetOwningPlayer<APTWPlayerController>())
	{
		PC->OnSpectateTargetChanged.RemoveAll(this);
	}
	
	Super::NativeDestruct();
}
