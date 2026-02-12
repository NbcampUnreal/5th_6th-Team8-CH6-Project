// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/LoadingScreen/PTWLoadingMiniGame.h"
#include "Components/TextBlock.h"

void UPTWLoadingMiniGame::SetupMiniGameInfo(const FText& InName, const FText& InDesc)
{
	if (MapNameText) MapNameText->SetText(InName);
	if (DescriptionText) DescriptionText->SetText(InDesc);
}
