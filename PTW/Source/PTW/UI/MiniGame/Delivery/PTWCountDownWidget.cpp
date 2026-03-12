// Fill out your copyright notice in the Description page of Project Settings.


#include "PTWCountDownWidget.h"

#include "Components/TextBlock.h"

void UPTWCountDownWidget::UpdateCountDown(int32 Count)
{
	if (Count > 0)
	{
		CountDownTextBlock->SetText(FText::AsNumber(Count));
		PlayAnimation(CountdownAnim);
	}
	else
	{
		CountDownTextBlock->SetText(FText::FromString("TIME UP!"));
	}
}
