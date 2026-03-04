// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/PTWButton.h"
#include "Components/TextBlock.h"
#include "Kismet/GameplayStatics.h"

void UPTWButton::NativePreConstruct()
{
	Super::NativePreConstruct();

	if (InnerText)
	{
		InnerText->SetText(ButtonText);
	}
}

void UPTWButton::NativeConstruct()
{
	Super::NativeConstruct();

	if (InnerButton)
	{
		InnerButton->OnClicked.AddUniqueDynamic(this, &UPTWButton::HandleOnClicked);
	}
}

void UPTWButton::NativeOnAddedToFocusPath(const FFocusEvent& InFocusEvent)
{
	Super::NativeOnAddedToFocusPath(InFocusEvent);
	ApplyFocusEffect(true);
}

void UPTWButton::NativeOnRemovedFromFocusPath(const FFocusEvent& InFocusEvent)
{
	Super::NativeOnRemovedFromFocusPath(InFocusEvent);
	ApplyFocusEffect(false);

	if (HoverSound)
	{
		UGameplayStatics::PlaySound2D(this, HoverSound);
	}
}

void UPTWButton::NativeOnMouseEnter(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent)
{
	Super::NativeOnMouseEnter(InGeometry, InMouseEvent);
	ApplyFocusEffect(true);

	if (HoverSound)
	{
		UGameplayStatics::PlaySound2D(this, HoverSound);
	}
}

void UPTWButton::NativeOnMouseLeave(const FPointerEvent& InMouseEvent)
{
	Super::NativeOnMouseLeave(InMouseEvent);
	ApplyFocusEffect(false);
}

void UPTWButton::HandleOnClicked()
{
	if (ClickSound)
	{
		UGameplayStatics::PlaySound2D(this, ClickSound);
	}
	
	OnClicked.Broadcast();
}

void UPTWButton::ApplyFocusEffect(bool bIsFocused)
{
	FVector2D TargetScale = bIsFocused ? FVector2D(1.15f, 1.15f) : FVector2D(1.0f, 1.0f);
	SetRenderScale(TargetScale);
}
