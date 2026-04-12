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
		
		// for (auto PlayerVoiceInfo : PlayerVoiceInfoList)
		// {
		// 	if (!PlayerVoiceVolumes.Contains(PlayerVoiceInfo.Key))
		// 	{
		// 		APlayerState* TargetPlayerState = nullptr;
		// 		for (APlayerState* PS : GetWorld()->GetGameState()->PlayerArray)
		// 		{
		// 			if (PlayerVoiceInfo.Key == PS->GetUniqueId().ToString())
		// 			{
		// 				TargetPlayerState = PS;
		// 				break;
		// 			}
		// 		}
		//		if (!TargetPlayerState) return;
		
		for (APlayerState* PS : GetWorld()->GetGameState()->PlayerArray)
		{
			UPTWVoiceVolume* PlayerVoiceVolume = CreateWidget<UPTWVoiceVolume>(this, VoiceVolumeClass);
			PlayerVoiceVolume->InitWidget(PS);
		
			USizeBox* SizeBox = NewObject<USizeBox>(this);
			SizeBox->AddChild(PlayerVoiceVolume);
			SizeBox->SetMinDesiredHeight(60.f);
		
			UVerticalBoxSlot* SizeBoxSlot = AddChildToVerticalBox(SizeBox);
			SizeBoxSlot->SetHorizontalAlignment(HAlign_Fill);
			SizeBoxSlot->SetVerticalAlignment(VAlign_Fill);
			
			if (PS == GetOwningPlayer()->GetPlayerState<APlayerState>())
			{
				// SizeBox->SetVisibility(ESlateVisibility::Collapsed);
			}
			FString UniqueId = PS->GetUniqueId().ToString();
			if (!PlayerVoiceVolumes.Contains(UniqueId))
			{
				PlayerVoiceVolumes.Add(UniqueId, PlayerVoiceVolume);
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
	for (int32 i = 1; i < GetChildrenCount(); i++)
	{
		GetChildAt(1)->RemoveFromParent();
	}
	PlayerVoiceVolumes.Empty();
	Super::ReleaseSlateResources(bReleaseChildren);
}
