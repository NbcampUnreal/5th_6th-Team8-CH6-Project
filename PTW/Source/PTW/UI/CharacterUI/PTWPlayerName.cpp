// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/CharacterUI/PTWPlayerName.h"
#include "Components/TextBlock.h"

void UPTWPlayerName::SetPlayerName(const FString& InName)
{
	if (PlayerNameText)
	{
		PlayerNameText->SetText(FText::FromString(InName));
	}
}
