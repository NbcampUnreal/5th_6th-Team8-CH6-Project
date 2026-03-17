// Fill out your copyright notice in the Description page of Project Settings.


#include "PTWDeliveryHUD.h"

#include "PTWBatterLevelWidget.h"
#include "PTWCountDownWidget.h"
#include "PTWRankingWidget.h"

void UPTWDeliveryHUD::InitBatterLevelWidget(UAbilitySystemComponent* ASC)
{
	BatterLevelWidget->InitWithASC(ASC);
	BatterLevelWidget->SetVisibility(ESlateVisibility::Visible);
}

void UPTWDeliveryHUD::InitCountDownWidget()
{
	CountDownWidget->SetVisibility(ESlateVisibility::Visible);
}

void UPTWDeliveryHUD::UpdateCountDownWidgetCount(int32 Count)
{
	CountDownWidget->UpdateCountDown(Count);
}

void UPTWDeliveryHUD::UpdateRank(int32 CurRank, int32 Total)
{
	if (RankingWidget)
	{
		RankingWidget->UpdateRank(CurRank, Total);
	}
	else 
	{
		UE_LOG(LogTemp, Warning, TEXT("RankingWidget is NULL!"));
	}
}


