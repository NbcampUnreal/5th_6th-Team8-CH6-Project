// Fill out your copyright notice in the Description page of Project Settings.


#include "PTWVoiceChatListWidget.h"
#include "PTWVoiceChatWidget.h"
#include "Components/SizeBox.h"
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
	if (PlayerNetId.IsEmpty() || !IsValid(VoiceChatList)) return;
	
	FString PlayerName = GetPlayerNameFromNetId(PlayerNetId);
	USizeBox* TargetWidget = nullptr;
    
	// VoiceChatList에 플레이어가 없으면 추가
	if (!PlayerVoiceChats.Contains(PlayerNetId))
	{
		TargetWidget = NewObject<USizeBox>(this, USizeBox::StaticClass());
		TargetWidget->SetHeightOverride(32.0f);
		
		UPTWVoiceChatWidget* VoiceChatWidget = CreateWidget<UPTWVoiceChatWidget>(this, VoiceChatWidgetClass);
		VoiceChatWidget->SetupWidget(PlayerName);
		TargetWidget->AddChild(VoiceChatWidget);
		
		PlayerVoiceChats.Add(PlayerNetId, TargetWidget);
		
		// 리스트 위젯에 추가
		VoiceChatList->AddChildToVerticalBox(TargetWidget);
	}
	else
	{
		TargetWidget = PlayerVoiceChats[PlayerNetId];
	}
	
	if (bIsTalking)
	{
		TargetWidget->SetVisibility(ESlateVisibility::SelfHitTestInvisible);
	}
	else
	{
		TargetWidget->SetVisibility(ESlateVisibility::Collapsed);
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
