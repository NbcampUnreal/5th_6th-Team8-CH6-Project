// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/MiniGame/AbilityBattle/PTWAbilityDraftWidget.h"

#include "Components/HorizontalBox.h"
#include "Components/HorizontalBoxSlot.h"
void UPTWAbilityDraftWidget::GenerateAbilityBoxes(int32 Count)
{
	if (!HorizontalBox || !AbilityBoxClass) return;

	for (int32 i = 0; i < Count; i++)
	{
		UUserWidget* NewWidget = CreateWidget<UUserWidget>(this, AbilityBoxClass);
		if (!NewWidget) return;

		UHorizontalBoxSlot* BoxSlot = HorizontalBox->AddChildToHorizontalBox(NewWidget);

		FSlateChildSize FillSize;
		FillSize.SizeRule = ESlateSizeRule::Fill;
		BoxSlot->SetSize(FillSize);

		BoxSlot->SetPadding(FMargin(15.f, 0.f)); 
		BoxSlot->SetVerticalAlignment(VAlign_Center);
	}
}
