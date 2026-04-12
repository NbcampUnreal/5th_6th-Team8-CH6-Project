#include "PTWVoiceVlumeVertical.h"
#include "PTWVoiceVolume.h"
#include "Components/SizeBox.h"
#include "Components/VerticalBoxSlot.h"
#include "GameFramework/GameStateBase.h"
#include "GameFramework/PlayerState.h"
#include "System/PTWVoiceChatSubsystem.h"


void UPTWVoiceVlumeVertical::InitWidget()
{
	if (UPTWVoiceChatSubsystem* VoiceChatSubsystem = UPTWVoiceChatSubsystem::Get(this))
	{
		TMap<FString, FPTWPlayerVoiceInfo>& PlayerVoiceInfoList = VoiceChatSubsystem->PlayerVoiceInfoList;
		if (!IsValid(VoiceVolumeClass)) return;
		
		for (APlayerState* PS : GetWorld()->GetGameState()->PlayerArray)
		{
			FString UniqueId = PS->GetUniqueId().ToString();
			if (!PlayerVoiceInfoList.Contains(UniqueId))
			{
				PlayerVoiceInfoList.Add(UniqueId, FPTWPlayerVoiceInfo());
			}
		}
		for (auto PlayerVoiceInfoEntry : PlayerVoiceInfoList)
		{
			if (!PlayerVoiceVolumes.Contains(PlayerVoiceInfoEntry.Key))
			{
				APlayerState* TargetPS = nullptr;
				for (APlayerState* PS : GetWorld()->GetGameState()->PlayerArray)
				{
					FString UniqueId = PS->GetUniqueId().ToString();
					if (PlayerVoiceInfoEntry.Key == UniqueId)
					{
						TargetPS = PS;
						break;
					}
				}
				if (!IsValid(TargetPS)) continue;
				
				UPTWVoiceVolume* PlayerVoiceVolume = CreateWidget<UPTWVoiceVolume>(this, VoiceVolumeClass);
				PlayerVoiceVolume->InitWidget(TargetPS);
		
				USizeBox* SizeBox = NewObject<USizeBox>(this);
				SizeBox->AddChild(PlayerVoiceVolume);
				SizeBox->SetMinDesiredHeight(60.f);
		
				UVerticalBoxSlot* SizeBoxSlot = AddChildToVerticalBox(SizeBox);
				SizeBoxSlot->SetHorizontalAlignment(HAlign_Fill);
				SizeBoxSlot->SetVerticalAlignment(VAlign_Fill);
			
				if (TargetPS == GetOwningPlayer()->GetPlayerState<APlayerState>())
				{
					SizeBox->SetVisibility(ESlateVisibility::Collapsed);
				}
				FString UniqueId = TargetPS->GetUniqueId().ToString();
				if (!PlayerVoiceVolumes.Contains(UniqueId))
				{
					PlayerVoiceVolumes.Add(UniqueId, PlayerVoiceVolume);
				}
			}
		}
	}
}

TSharedRef<SWidget> UPTWVoiceVlumeVertical::RebuildWidget()
{
	InitWidget();
	return Super::RebuildWidget();
}

void UPTWVoiceVlumeVertical::ReleaseSlateResources(bool bReleaseChildren)
{
	Super::ReleaseSlateResources(bReleaseChildren);
}
