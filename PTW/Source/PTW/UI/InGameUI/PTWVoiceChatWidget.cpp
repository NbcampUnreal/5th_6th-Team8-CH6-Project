// Fill out your copyright notice in the Description page of Project Settings.


#include "PTWVoiceChatWidget.h"

#include "Components/TextBlock.h"
#include "CoreFramework/PTWPlayerController.h"
#include "GameFramework/PlayerState.h"
#include "System/PTWVoiceChatSubsystem.h"

void UPTWVoiceChatWidget::NativeConstruct()
{
	Super::NativeConstruct();
}

void UPTWVoiceChatWidget::NativeDestruct()
{
	Super::NativeDestruct();
}

void UPTWVoiceChatWidget::SetupWidget(FString InTalkerName)
{
	TalkerName = InTalkerName;
	TalkerNameText->SetText(FText::FromString(TalkerName));
}
