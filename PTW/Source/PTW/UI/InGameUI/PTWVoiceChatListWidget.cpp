#include "PTWVoiceChatListWidget.h"
#include "PTWVoiceChatWidget.h"
#include "Components/SizeBox.h"
#include "Components/VerticalBox.h"
#include "CoreFramework/PTWPlayerController.h"
#include "GameFramework/PlayerState.h"
#include "System/PTWVoiceChatSubsystem.h"


void UPTWVoiceChatListWidget::InitializeWidget(const FString& InUniqueId)
{
	UPTWVoiceChatSubsystem* VoiceChatSubsystem = UPTWVoiceChatSubsystem::Get(this);
	if (!IsValid(VoiceChatSubsystem)) return;
	
	VoiceChatSubsystem->OnVoiceChatConnected.AddUniqueDynamic(this, &ThisClass::HandlePlayerConnected);
	VoiceChatSubsystem->OnVoiceChatDisconnected.AddUniqueDynamic(this, &ThisClass::HandlePlayerDisconnected);
	VoiceChatSubsystem->AllVoiceChatDisconnected.AddUniqueDynamic(this, &ThisClass::HandleAllPlayersConnected);
	VoiceChatSubsystem->OnVoiceStateUpdated.AddUniqueDynamic(this, &ThisClass::OnVoiceStateChanged);
	
	VoiceChatSubsystem->OnLocalVoiceChatConnected.RemoveDynamic(this, &ThisClass::InitializeWidget);
	
	TMap<FString, FPTWPlayerVoiceInfo>& PlayerVoiceInfoList = VoiceChatSubsystem->PlayerVoiceInfoList;
	for (auto PlayerVoiceInfoEnty : PlayerVoiceInfoList)
	{
		const FString& UniqueId =  PlayerVoiceInfoEnty.Key;
		HandlePlayerConnected(UniqueId);
	}
}

void UPTWVoiceChatListWidget::OnVoiceStateChanged(const FString& UniqueId, bool bIsTalking)
{
	if (UniqueId.IsEmpty() || !IsValid(VoiceChatList)) return;
	
	UPTWVoiceChatSubsystem* VoiceChatSubsystem = UPTWVoiceChatSubsystem::Get(this);
	if (!IsValid(VoiceChatSubsystem)) return;
	
	TMap<FString, FPTWPlayerVoiceInfo>& PlayerVoiceInfoList = VoiceChatSubsystem->PlayerVoiceInfoList;
	if (!PlayerVoiceInfoList.Contains(UniqueId)) return;
	
	if (!PlayerVoiceChats.Contains(UniqueId)) return;
	
	UPTWVoiceChatWidget* TargetWidget = PlayerVoiceChats[UniqueId];

	const FString& LocalUniqueId = GetOwningPlayerState()->GetUniqueId().ToString();
	if (LocalUniqueId == UniqueId)
	{
		if (bIsTalking)
		{
			TargetWidget->SetTalkingVoiceIcon();
		}
		else
		{
			TargetWidget->SetEnabledVoiceIcon();
		}	
	}
	else
	{
		if (UPanelWidget* ParentWrapper = TargetWidget->GetParent())
		{
			ESlateVisibility NewVisibility = bIsTalking ? ESlateVisibility::SelfHitTestInvisible : ESlateVisibility::Collapsed;
			ParentWrapper->SetVisibility(NewVisibility);
		}
	}
}

void UPTWVoiceChatListWidget::HandleChangedVoiceChatState(bool bIsActive)
{
	FString UniqueId = GetOwningPlayerState()->GetUniqueId().ToString();
	
	if (!PlayerVoiceChats.Contains(UniqueId)) return;
	
	UPTWVoiceChatWidget* TargetWidget = PlayerVoiceChats[UniqueId];
	if (!IsValid(TargetWidget)) return;
	
	if (UPanelWidget* ParentWrapper = TargetWidget->GetParent())
	{
		ESlateVisibility NewVisibility = bIsActive ? ESlateVisibility::SelfHitTestInvisible : ESlateVisibility::Collapsed;
		ParentWrapper->SetVisibility(NewVisibility);
	}
}

void UPTWVoiceChatListWidget::NativeConstruct()
{
	Super::NativeConstruct();
	
	if (UPTWVoiceChatSubsystem* VoiceChatSubsystem = UPTWVoiceChatSubsystem::Get(this))
	{
		VoiceChatSubsystem->OnLocalVoiceChatConnected.AddUniqueDynamic(this, &ThisClass::InitializeWidget);
	}
	
	if (APTWPlayerController* PC = GetOwningPlayer<APTWPlayerController>())
	{
		PC->OnChangedVoiceChatState.AddUniqueDynamic(this, &ThisClass::HandleChangedVoiceChatState);
	}
	if (GetOwningPlayerState() && GetOwningPlayerState()->GetUniqueId().IsValid() && 
		!GetOwningPlayerState()->GetPlayerName().IsEmpty() && GetOwningPlayerState()->GetPlayerName() != TEXT("Player"))
	{
		InitializeWidget(GetOwningPlayerState()->GetUniqueId().ToString());
	}
}

void UPTWVoiceChatListWidget::NativeDestruct()
{
	if (UPTWVoiceChatSubsystem* VoiceChatSubsystem = UPTWVoiceChatSubsystem::Get(this))
	{
		VoiceChatSubsystem->OnLocalVoiceChatConnected.RemoveDynamic(this, &ThisClass::InitializeWidget);
		VoiceChatSubsystem->OnVoiceChatConnected.RemoveDynamic(this, &ThisClass::HandlePlayerConnected);
		VoiceChatSubsystem->OnVoiceChatDisconnected.RemoveDynamic(this, &ThisClass::HandlePlayerDisconnected);
		VoiceChatSubsystem->AllVoiceChatDisconnected.RemoveDynamic(this, &ThisClass::HandleAllPlayersConnected);
		
		VoiceChatSubsystem->OnVoiceStateUpdated.RemoveDynamic(this, &ThisClass::OnVoiceStateChanged);
	}
	if (APTWPlayerController* PC = GetOwningPlayer<APTWPlayerController>())
	{
		PC->OnChangedVoiceChatState.RemoveDynamic(this, &ThisClass::HandleChangedVoiceChatState);
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
		VoiceChatList->AddChildToVerticalBox(SizeBox);
	}
	else
	{
		TargetWidget = PlayerVoiceChats[UniqueId];
	}
	
	const FString& LocalUniqueId = GetOwningPlayerState()->GetUniqueId().ToString();
	if (LocalUniqueId == UniqueId)
	{
		TargetWidget->SetEnabledVoiceIcon();
	}
	else
	{
		TargetWidget->SetTalkingVoiceIcon();
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
			PlayerVoiceChats.Remove(UniqueId);
			ParentWrapper->RemoveFromParent();
		}
	}
}

void UPTWVoiceChatListWidget::HandleAllPlayersConnected()
{
	VoiceChatList->ClearChildren();
	PlayerVoiceChats.Empty();
}
