#include "PTWVoiceVlumeVertical.h"
#include "PTWVoiceVolume.h"
#include "Components/SizeBox.h"
#include "Components/VerticalBoxSlot.h"
#include "System/PTWVoiceChatSubsystem.h"


void UPTWVoiceVlumeVertical::InitializeWidget()
{
	if (UPTWVoiceChatSubsystem* VoiceChatSubsystem = UPTWVoiceChatSubsystem::Get(this))
	{
		TMap<FString, FPTWPlayerVoiceInfo>& PlayerVoiceInfoList = VoiceChatSubsystem->PlayerVoiceInfoList;
		if (!IsValid(VoiceVolumeClass)) return;
		
		for (auto It = PlayerVoiceVolumes.CreateIterator(); It; ++It)
		{
			const FString& UniqueId = It.Key();
			if (!PlayerVoiceInfoList.Contains(UniqueId))
			{
				if (!IsValid(It.Value()))
				{
					It.RemoveCurrent();
					continue;
				}
				
				if (UPanelWidget* ParentWrapper = It.Value()->GetParent())
				{
					ParentWrapper->RemoveFromParent();
				}
				It.RemoveCurrent();
			}
		}
		
		for (auto PlayerVoiceInfoEntry : PlayerVoiceInfoList)
		{
			const FString& UniqueId = PlayerVoiceInfoEntry.Key;
			const FPTWPlayerVoiceInfo& PlayerVoiceInfo = PlayerVoiceInfoEntry.Value;
			if (!PlayerVoiceVolumes.Contains(UniqueId))
			{
				UPTWVoiceVolume* PlayerVoiceVolume = CreateWidget<UPTWVoiceVolume>(this, VoiceVolumeClass);
				PlayerVoiceVolume->InitWidget(UniqueId, PlayerVoiceInfo);
				
				USizeBox* SizeBox = NewObject<USizeBox>(this);
				SizeBox->AddChild(PlayerVoiceVolume);
				SizeBox->SetMinDesiredHeight(60.f);
		
				UVerticalBoxSlot* SizeBoxSlot = AddChildToVerticalBox(SizeBox);
				SizeBoxSlot->SetHorizontalAlignment(HAlign_Fill);
				SizeBoxSlot->SetVerticalAlignment(VAlign_Fill);
				
				const FString& MyUniqueId = GetOwningLocalPlayer()->GetPreferredUniqueNetId().ToString();
				if (UniqueId == MyUniqueId)
				{
					// SizeBox->SetVisibility(ESlateVisibility::Collapsed);
				}
				
				PlayerVoiceVolumes.Add(UniqueId, PlayerVoiceVolume);
			}
		}
	}
}

void UPTWVoiceVlumeVertical::HandleVoiceChatDisconnected()
{
	
}

TSharedRef<SWidget> UPTWVoiceVlumeVertical::RebuildWidget()
{
	InitializeWidget();
	return Super::RebuildWidget();
}

void UPTWVoiceVlumeVertical::ReleaseSlateResources(bool bReleaseChildren)
{
	Super::ReleaseSlateResources(bReleaseChildren);
}
