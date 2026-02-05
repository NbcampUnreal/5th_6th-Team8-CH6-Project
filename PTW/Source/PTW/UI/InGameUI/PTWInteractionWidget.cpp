// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/InGameUI/PTWInteractionWidget.h"
#include "Components/TextBlock.h"
#include "GameFramework/Pawn.h"
#include "CoreFramework/Character/Component/PTWInteractComponent.h"

void UPTWInteractionWidget::NativeConstruct()
{
	Super::NativeConstruct();

	SetVisibility(ESlateVisibility::Hidden);

	APawn* OwnerPawn = GetOwningPlayerPawn();
	if (!OwnerPawn) return;

	UPTWInteractComponent* InteractComp = OwnerPawn->FindComponentByClass<UPTWInteractComponent>();

	if (!InteractComp) return;

	/* 델리게이트 바인딩 */
	InteractComp->OnInteractableFound.AddDynamic(this, &UPTWInteractionWidget::ShowWithText);
	InteractComp->OnInteractableLost.AddDynamic(this, &UPTWInteractionWidget::Hide);
}

void UPTWInteractionWidget::NativeDestruct()
{
	APawn* OwnerPawn = GetOwningPlayerPawn();
	if (OwnerPawn)
	{
		if (UPTWInteractComponent* InteractComp = OwnerPawn->FindComponentByClass<UPTWInteractComponent>())
		{
			InteractComp->OnInteractableFound.RemoveDynamic(this, &UPTWInteractionWidget::ShowWithText);
			InteractComp->OnInteractableLost.RemoveDynamic(this, &UPTWInteractionWidget::Hide);
		}
	}

	Super::NativeDestruct();
}

void UPTWInteractionWidget::ShowWithText(const FText& InText)
{
	if (ActionText)
	{
		ActionText->SetText(InText);
	}

	SetVisibility(ESlateVisibility::Visible);
}

void UPTWInteractionWidget::Hide()
{
	SetVisibility(ESlateVisibility::Hidden);
}
