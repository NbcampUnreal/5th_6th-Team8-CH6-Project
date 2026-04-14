#include "PTWVoiceChatListWidget.h"
#include "PTWVoiceChatWidget.h"
#include "Components/SizeBox.h"
#include "Components/VerticalBox.h"
#include "CoreFramework/PTWPlayerController.h"
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
	
	if (APTWPlayerController* PC = GetOwningPlayer<APTWPlayerController>())
	{
		PC->OnChangedVoiceChatState.AddUniqueDynamic(this, &ThisClass::HandleChangedVoiceChatState);
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
	
	if (APTWPlayerController* PC = GetOwningPlayer<APTWPlayerController>())
	{
		PC->OnChangedVoiceChatState.RemoveDynamic(this, &ThisClass::HandleChangedVoiceChatState);
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
	UPTWVoiceChatWidget* TargetWidget = nullptr;
    
	// VoiceChatList에 플레이어가 없으면 추가
	if (!PlayerVoiceChats.Contains(PlayerNetId))
	{
		USizeBox* SizeBox = NewObject<USizeBox>(this, USizeBox::StaticClass());
		SizeBox->SetHeightOverride(32.0f);
		
		TargetWidget = CreateWidget<UPTWVoiceChatWidget>(this, VoiceChatWidgetClass);
		TargetWidget->SetupWidget(PlayerName);
		SizeBox->AddChild(TargetWidget);
		
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
		TargetWidget->SetTalkingVoiceIcon();
	}
	else
	{
		TargetWidget->SetEnabledVoiceIcon();
	}
	
	if (GetOwningPlayerState()->GetUniqueId().ToString() != PlayerNetId)
	{
		if (UPanelWidget* ParentWrapper = TargetWidget->GetParent())
		{
			ESlateVisibility NewVisibility = bIsTalking ? ESlateVisibility::SelfHitTestInvisible : ESlateVisibility::Collapsed;
			ParentWrapper->SetVisibility(NewVisibility);
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

void UPTWVoiceChatListWidget::HandleChangedVoiceChatState(bool bIsActive)
{
	FString PlayerNetId = GetOwningPlayerState()->GetUniqueId().ToString();
	FString PlayerName = GetPlayerNameFromNetId(PlayerNetId);
	UPTWVoiceChatWidget* TargetWidget = nullptr;
    
	// VoiceChatList에 플레이어가 없으면 추가
	if (!PlayerVoiceChats.Contains(PlayerNetId))
	{
		USizeBox* SizeBox = NewObject<USizeBox>(this, USizeBox::StaticClass());
		SizeBox->SetHeightOverride(32.0f);
		
		TargetWidget = CreateWidget<UPTWVoiceChatWidget>(this, VoiceChatWidgetClass);
		TargetWidget->SetupWidget(PlayerName);
		SizeBox->AddChild(TargetWidget);
		
		PlayerVoiceChats.Add(PlayerNetId, TargetWidget);
		
		// 리스트 위젯에 추가
		VoiceChatList->AddChildToVerticalBox(TargetWidget);
	}
	else
	{
		TargetWidget = PlayerVoiceChats[PlayerNetId];
	}

	if (UPanelWidget* ParentWrapper = TargetWidget->GetParent())
	{
		ESlateVisibility NewVisibility = bIsActive ? ESlateVisibility::SelfHitTestInvisible : ESlateVisibility::Collapsed;
		ParentWrapper->SetVisibility(NewVisibility);
	}
}
