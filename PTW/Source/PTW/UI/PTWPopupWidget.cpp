// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/PTWPopupWidget.h"
#include "Components/TextBlock.h"
#include "Components/Button.h"
#include "UI/PTWUISubsystem.h"

void UPTWPopupWidget::NativeConstruct()
{
	Super::NativeConstruct();

	bIsFocusable = true;

	if (OkButton)
	{
		OkButton->OnClicked.AddDynamic(this, &ThisClass::OnClickedOK);
	}
}

void UPTWPopupWidget::SetMessage(const FText& InText)
{
	if (MessageText)
	{
		MessageText->SetText(InText);
	}
}

void UPTWPopupWidget::OnClickedOK()
{
	if (ULocalPlayer* LP = GetOwningLocalPlayer())
	{
		if (UPTWUISubsystem* UISub = LP->GetSubsystem<UPTWUISubsystem>())
		{
			UISub->PopWidget();
		}
	}
}
