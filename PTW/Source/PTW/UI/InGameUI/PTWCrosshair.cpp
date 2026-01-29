// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/InGameUI/PTWCrosshair.h"

void UPTWCrosshair::SetCrosshairVisibility(bool bVisible)
{
	SetVisibility(bVisible ? ESlateVisibility::Visible : ESlateVisibility::Hidden);
}

void UPTWCrosshair::NativeConstruct()
{
	Super::NativeConstruct();
	SetVisibility(ESlateVisibility::Hidden);
}

