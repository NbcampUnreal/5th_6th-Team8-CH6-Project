// Fill out your copyright notice in the Description page of Project Settings.


#include "PTWVoiceChatListWidget.h"
#include "PTWVoiceChatWidget.h"
#include "Components/VerticalBox.h"
#include "GameFramework/GameStateBase.h"
#include "GameFramework/PlayerState.h"
#include "System/PTWVoiceChatSubsystem.h"

void UPTWVoiceChatListWidget::NativeConstruct()
{
	Super::NativeConstruct();
	
	if (ULocalPlayer* LocalPlayer = GetOwningLocalPlayer())
	{
		if (UPTWVoiceChatSubsystem* VoiceChatSubsystem = LocalPlayer->GetSubsystem<UPTWVoiceChatSubsystem>())
		{
			VoiceChatSubsystem->OnVoiceStateUpdated.AddUniqueDynamic(this, &ThisClass::OnVoiceStateChanged);
		}
	}
}

void UPTWVoiceChatListWidget::NativeDestruct()
{
	if (ULocalPlayer* LocalPlayer = GetOwningLocalPlayer())
	{
		if (UPTWVoiceChatSubsystem* VoiceChatSubsystem = LocalPlayer->GetSubsystem<UPTWVoiceChatSubsystem>())
		{
			if (VoiceChatSubsystem->OnVoiceStateUpdated.IsAlreadyBound(this, &ThisClass::OnVoiceStateChanged))
			{
				VoiceChatSubsystem->OnVoiceStateUpdated.RemoveDynamic(this, &ThisClass::OnVoiceStateChanged);
			}
		}
	}
	
	Super::NativeDestruct();
}

void UPTWVoiceChatListWidget::Init()
{
	
}

void UPTWVoiceChatListWidget::OnVoiceStateChanged(const FString& PlayerNetId, bool bIsTalking)
{
	if (PlayerNetId.IsEmpty() || !IsValid(VoiceChatListVerticalBox)) return;
	
	FString PlayerName = GetPlayerNameFromNetId(PlayerNetId);
	UPTWVoiceChatWidget* TargetWidget = nullptr;
    
	for (UPTWVoiceChatWidget* Widget : VoiceChatWidgets)
	{
		if (IsValid(Widget) && PlayerName == Widget->GetTalkerName())
		{
			TargetWidget = Widget;
			break;
		}
	}
	
	if (!IsValid(TargetWidget) && IsValid(VoiceChatWidgetClass))
	{
		TargetWidget = CreateWidget<UPTWVoiceChatWidget>(this, VoiceChatWidgetClass);
		TargetWidget->SetupWidget(PlayerName);
		VoiceChatWidgets.Add(TargetWidget);
	}
	
	if (!IsValid(TargetWidget)) return;
	
	if (bIsTalking)
	{
		if (!VoiceChatListVerticalBox->HasChild(TargetWidget))
		{
			VoiceChatListVerticalBox->AddChildToVerticalBox(TargetWidget);
		}
	}
	else
	{
		if (VoiceChatListVerticalBox->HasChild(TargetWidget))
		{
			VoiceChatListVerticalBox->RemoveChild(TargetWidget);
		}
	}
}

FString UPTWVoiceChatListWidget::GetPlayerNameFromNetId(const FString& TargetNetId)
{
	UWorld* World = GetWorld();
	if (!World) return TEXT("Unknown");
	
	AGameStateBase* GameState = World->GetGameState();
	if (!GameState) return TEXT("Unknown");
	
	for (APlayerState* PS : GameState->PlayerArray)
	{
		if (IsValid(PS))
		{
			const FUniqueNetIdRepl& TempNetId = PS->GetUniqueId();
			if (TempNetId.IsValid() && TempNetId->ToString() == TargetNetId)
			{
				return PS->GetPlayerName();
			}
		}
	}
	return TEXT("Unknown");
}
