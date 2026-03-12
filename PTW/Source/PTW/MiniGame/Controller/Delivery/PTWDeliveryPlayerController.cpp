// Fill out your copyright notice in the Description page of Project Settings.


#include "PTWDeliveryPlayerController.h"

#include "UI/PTWUISubsystem.h"
#include "UI/MiniGame/Delivery/PTWBatterLevelWidget.h"

void APTWDeliveryPlayerController::CreateUI()
{
	Super::CreateUI();
	
	if (UISubsystem)
	{
		UUserWidget* WidgetInstance = UISubsystem->CreatePersistentWidget(BatteryWidgetClass);
		if (UPTWBatterLevelWidget* BatterLevelWidget = Cast<UPTWBatterLevelWidget>(WidgetInstance))
		{
			BatterLevelWidget->InitWithASC(UISubsystem->GetLocalPlayerASC());
			BatterLevelWidget->SetVisibility(ESlateVisibility::Visible);
		}
	}
}
