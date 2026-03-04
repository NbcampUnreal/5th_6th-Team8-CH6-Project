 // Fill out your copyright notice in the Description page of Project Settings.


#include "UI/InGameUI/PTWNotificationWidget.h"
#include "Components/TextBlock.h"

void UPTWNotificationWidget::PlayMessage(const FNotificationData& Data)
{
	SetVisibility(ESlateVisibility::Visible);

	if (MessageText)
	{
		MessageText->SetText(Data.Message);
	}

	ApplyStyle(Data.Priority);

	GetWorld()->GetTimerManager().SetTimer(
		MessageTimerHandle,
		this,
		&UPTWNotificationWidget::HandleFinish,
		Data.Duration,
		false
	);
}

void UPTWNotificationWidget::ForceHide()
{
	GetWorld()->GetTimerManager().ClearTimer(MessageTimerHandle);
	SetVisibility(ESlateVisibility::Hidden);
	OnMessageFinished.Broadcast();
}

void UPTWNotificationWidget::NativeConstruct()
{
	Super::NativeConstruct();
	SetVisibility(ESlateVisibility::Hidden);
}

void UPTWNotificationWidget::ApplyStyle(ENotificationPriority InPriority)
{
	// 필요할 때 추가

	/**************************************
	if (!MessageText) return;

	FLinearColor Color = FLinearColor::White;

	switch (InPriority)
	{
	case ENotificationPriority::Low:
		Color = FLinearColor::Gray;
		break;

	case ENotificationPriority::Normal:
		Color = FLinearColor::White;
		break;

	case ENotificationPriority::High:
		Color = FLinearColor::Yellow;
		break;

	case ENotificationPriority::Critical:
		Color = FLinearColor::Red;
		break;
	}

	MessageText->SetColorAndOpacity(FSlateColor(Color));
	*****************************/
}

void UPTWNotificationWidget::HandleFinish()
{
	SetVisibility(ESlateVisibility::Hidden);
	OnMessageFinished.Broadcast();
}
