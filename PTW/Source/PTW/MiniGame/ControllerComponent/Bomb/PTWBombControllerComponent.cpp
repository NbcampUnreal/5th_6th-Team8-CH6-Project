// Fill out your copyright notice in the Description page of Project Settings.


#include "MiniGame/ControllerComponent/Bomb/PTWBombControllerComponent.h"

#include "MiniGame/Item/BombItem/PTWBombActor.h"
#include "UI/MiniGame/Bomb/PTWBombWarning.h"

#include "CoreFramework/PTWPlayerController.h"
#include "UI/PTWUISubsystem.h"

void UPTWBombControllerComponent::BindBombDelegate(APTWBombActor* NewBomb)
{
	APTWPlayerController* PTWPC = Cast<APTWPlayerController>(GetWorld()->GetFirstPlayerController());
	if (!PTWPC) return;

	UISubsystem = PTWPC->GetUISubSystem();

	// 중복 바인드 방지
	UnBindBombDelegate();

	CachedBombActor = NewBomb;

	if (CachedBombActor)
	{
		CachedBombActor->OnBombOwnerChanged.AddUObject(this, &UPTWBombControllerComponent::HandleBombOwnerChanged);
	}

	if (!BombWarningWidgetClass || !UISubsystem) return;

	UUserWidget* Widget = UISubsystem->ShowSystemWidget(BombWarningWidgetClass, 70);
	if (UPTWBombWarning* BombWidget = Cast<UPTWBombWarning>(Widget))
	{
		BombWidget->SetTargetBomb(CachedBombActor);
	}

	UISubsystem->SetWidgetVisibility(BombWarningWidgetClass, false);

	if (CachedBombActor && CachedBombActor->GetBombOwnerPawn())
	{
		HandleBombOwnerChanged(CachedBombActor->GetBombOwnerPawn());
	}
}

void UPTWBombControllerComponent::UnBindBombDelegate()
{
	if (CachedBombActor)
	{
		CachedBombActor->OnBombOwnerChanged.RemoveAll(this);
		CachedBombActor = nullptr;
	}

	if (!BombWarningWidgetClass || !UISubsystem) return;

	UISubsystem->HideSystemWidget(BombWarningWidgetClass);
}

void UPTWBombControllerComponent::HandleBombOwnerChanged(APawn* NewOwnerPawn)
{
	APTWPlayerController* PTWPC = Cast<APTWPlayerController>(GetWorld()->GetFirstPlayerController());
	if (!PTWPC) return;

	if (NewOwnerPawn == PTWPC->GetPawn())
	{
		ShowBombUI();
	}
	else
	{
		HideBombUI();
	}
}

void UPTWBombControllerComponent::ShowBombUI()
{
	if (!BombWarningWidgetClass || !UISubsystem) return;

	UISubsystem->SetWidgetVisibility(BombWarningWidgetClass, true);
}

void UPTWBombControllerComponent::HideBombUI()
{
	if (!BombWarningWidgetClass || !UISubsystem) return;

	UISubsystem->SetWidgetVisibility(BombWarningWidgetClass, false);
}
