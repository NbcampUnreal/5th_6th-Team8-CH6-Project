// Fill out your copyright notice in the Description page of Project Settings.


#include "PTWRankingWidget.h"

#include "CoreFramework/PTWPlayerController.h"

void UPTWRankingWidget::NativeConstruct()
{
	Super::NativeConstruct();
}

void UPTWRankingWidget::UpdateRank(int32 NewRank, int32 Total)
{
	FString RankString = FString::Printf(TEXT("%d / %d"), NewRank, Total);
	RankingTextBlock->SetText(FText::FromString(RankString));
}
