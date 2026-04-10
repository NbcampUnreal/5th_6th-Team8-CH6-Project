// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/Predict/PTWPredictPlayerEntry.h"
#include "Components/TextBlock.h"
#include "Components/Button.h"

void UPTWPredictPlayerEntry::SetEntryData(const FString& InPlayerName)
{
	TargetPlayerName = InPlayerName;
	if (Text_Name)
	{
		Text_Name->SetText(FText::FromString(InPlayerName));
	}

	if (Btn_Player)
	{
		// 중복 방지
		Btn_Player->OnClicked.RemoveDynamic(this, &UPTWPredictPlayerEntry::OnBtnClicked);
		// 바인딩 시도
		Btn_Player->OnClicked.AddDynamic(this, &UPTWPredictPlayerEntry::OnBtnClicked);
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("Btn_Player is NULL! Check Widget Binding in Blueprint."));
	}
}

void UPTWPredictPlayerEntry::OnBtnClicked()
{
	OnClickedDelegate.ExecuteIfBound(TargetPlayerName);
}
