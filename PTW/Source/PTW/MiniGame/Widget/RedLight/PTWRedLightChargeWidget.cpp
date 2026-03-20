// Fill out your copyright notice in the Description page of Project Settings.


#include "MiniGame/Widget/RedLight/PTWRedLightChargeWidget.h"
#include "Components/ProgressBar.h"
#include "MiniGame/Character/RedLight/PTWRedLightCharacter.h"

void UPTWRedLightChargeWidget::NativeConstruct()
{
	Super::NativeConstruct();

	if (APawn* OwningPawn = GetOwningPlayerPawn())
	{
		CachedRedLightChar = Cast<APTWRedLightCharacter>(OwningPawn);
	}
}

void UPTWRedLightChargeWidget::NativeTick(const FGeometry& MyGeometry, float InDeltaTime)
{
	Super::NativeTick(MyGeometry, InDeltaTime);

	if (CachedRedLightChar.IsValid() && ChargeProgressBar)
	{
		float CurrentProgress = CachedRedLightChar->GetChargeProgress();
		ChargeProgressBar->SetPercent(CurrentProgress);
	}
}
