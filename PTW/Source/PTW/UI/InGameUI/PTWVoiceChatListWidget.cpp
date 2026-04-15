#include "PTWVoiceChatListWidget.h"
#include "PTWVoiceChatWidget.h"
#include "Components/SizeBox.h"
#include "Components/VerticalBox.h"
#include "CoreFramework/PTWPlayerController.h"
#include "CoreFramework/Game/GameInstance/PTWGameInstance.h"
#include "GameFramework/GameStateBase.h"
#include "GameFramework/PlayerState.h"
#include "System/PTWVoiceChatSubsystem.h"


void UPTWVoiceChatListWidget::Init()
{
	UPTWVoiceChatSubsystem* VoiceChatSubsystem = UPTWVoiceChatSubsystem::Get(this);
	if (!IsValid(VoiceChatSubsystem)) return;
	
	TMap<FString, FPTWPlayerVoiceInfo>& PlayerVoiceInfoList = VoiceChatSubsystem->PlayerVoiceInfoList;
	
	for (auto PlayerVoiceInfoEnty : PlayerVoiceInfoList)
	{
		const FString& UniqueId =  PlayerVoiceInfoEnty.Key;
		const FPTWPlayerVoiceInfo& PlayerVoiceInfo =  PlayerVoiceInfoEnty.Value;
		
		UPTWVoiceChatWidget* TargetWidget = nullptr;
		if (!PlayerVoiceChats.Contains(UniqueId))
		{
			USizeBox* SizeBox = NewObject<USizeBox>(this, USizeBox::StaticClass());
			SizeBox->SetHeightOverride(32.0f);
		
			TargetWidget = CreateWidget<UPTWVoiceChatWidget>(this, VoiceChatWidgetClass);
			TargetWidget->SetupWidget(PlayerVoiceInfo.PlayerName);
			SizeBox->AddChild(TargetWidget);
		
			PlayerVoiceChats.Add(UniqueId, TargetWidget);
			VoiceChatList->AddChildToVerticalBox(TargetWidget);
		}
		else
		{
			TargetWidget = PlayerVoiceChats[UniqueId];
		}

		if (UPanelWidget* ParentWrapper = TargetWidget->GetParent())
		{
			ParentWrapper->SetVisibility(ESlateVisibility::Collapsed);
		}
	}
}

void UPTWVoiceChatListWidget::OnVoiceStateChanged(const FString& PlayerNetId, bool bIsTalking)
{
	if (PlayerNetId.IsEmpty() || !IsValid(VoiceChatList)) return;
	
	FString PlayerName = GetPlayerNameFromNetId(PlayerNetId);
	if (!PlayerVoiceChats.Contains(PlayerNetId)) return;
	
	UPTWVoiceChatWidget* TargetWidget = PlayerVoiceChats[PlayerNetId];
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
	
	if (UPTWGameInstance* GI = GetGameInstance<UPTWGameInstance>())
	{
		GI->OnPlayerEnteredLevel.AddUniqueDynamic(this, &ThisClass::HandlePlayerConnected);
		GI->OnPlayerLeftLevel.AddUniqueDynamic(this, &ThisClass::HandlePlayerDisconnected);
		GI->OnLevelPlayersCleared.AddUniqueDynamic(this, &ThisClass::HandleAllPlayersConnected);
	}
	
	Init();
}

void UPTWVoiceChatListWidget::NativeDestruct()
{
	if (ULocalPlayer* LocalPlayer = GetOwningLocalPlayer())
	{
		if (UPTWVoiceChatSubsystem* VoiceChatSubsystem = LocalPlayer->GetSubsystem<UPTWVoiceChatSubsystem>())
		{
			VoiceChatSubsystem->OnVoiceStateUpdated.RemoveDynamic(this, &ThisClass::OnVoiceStateChanged);
		}
	}
	
	if (APTWPlayerController* PC = GetOwningPlayer<APTWPlayerController>())
	{
		PC->OnChangedVoiceChatState.RemoveDynamic(this, &ThisClass::HandleChangedVoiceChatState);
	}
	
	if (UPTWGameInstance* GI = GetGameInstance<UPTWGameInstance>())
	{
		GI->OnPlayerEnteredLevel.RemoveDynamic(this, &ThisClass::HandlePlayerConnected);
		GI->OnPlayerLeftLevel.RemoveDynamic(this, &ThisClass::HandlePlayerDisconnected);
		GI->OnLevelPlayersCleared.RemoveDynamic(this, &ThisClass::HandleAllPlayersConnected);
	}
	
	Super::NativeDestruct();
}

void UPTWVoiceChatListWidget::HandlePlayerConnected(const FString& UniqueId)
{
	UPTWVoiceChatSubsystem* VoiceChatSubsystem = UPTWVoiceChatSubsystem::Get(this);
	if (!IsValid(VoiceChatSubsystem)) return;
	
	TMap<FString, FPTWPlayerVoiceInfo>& PlayerVoiceInfoList = VoiceChatSubsystem->PlayerVoiceInfoList;
	if (!PlayerVoiceInfoList.Contains(UniqueId)) return;
	
	const FPTWPlayerVoiceInfo& PlayerVoiceInfo =  PlayerVoiceInfoList[UniqueId];
		
	UPTWVoiceChatWidget* TargetWidget = nullptr;
	if (!PlayerVoiceChats.Contains(UniqueId))
	{
		USizeBox* SizeBox = NewObject<USizeBox>(this, USizeBox::StaticClass());
		SizeBox->SetHeightOverride(32.0f);
	
		TargetWidget = CreateWidget<UPTWVoiceChatWidget>(this, VoiceChatWidgetClass);
		TargetWidget->SetupWidget(PlayerVoiceInfo.PlayerName);
		SizeBox->AddChild(TargetWidget);
	
		PlayerVoiceChats.Add(UniqueId, TargetWidget);
		VoiceChatList->AddChildToVerticalBox(TargetWidget);
	}
	else
	{
		TargetWidget = PlayerVoiceChats[UniqueId];
	}

	if (UPanelWidget* ParentWrapper = TargetWidget->GetParent())
	{
		ParentWrapper->SetVisibility(ESlateVisibility::Collapsed);
	}
}

void UPTWVoiceChatListWidget::HandlePlayerDisconnected(const FString& UniqueId)
{
	if (PlayerVoiceChats.Contains(UniqueId))
	{
		if (UPanelWidget* ParentWrapper = PlayerVoiceChats[UniqueId]->GetParent())
		{
			ParentWrapper->RemoveFromParent();
		}
	}
}

void UPTWVoiceChatListWidget::HandleAllPlayersConnected()
{
	VoiceChatList->ClearChildren();
	PlayerVoiceChats.Empty();
}
