#include "UI/OptionsWidget/PTWVoiceVolume.h"
#include "Components/EditableText.h"
#include "Components/Slider.h"
#include "Components/TextBlock.h"
#include "System/PTWVoiceChatSubsystem.h"

void UPTWVoiceVolume::InitWidget(const FString& UniqueId, const FPTWPlayerVoiceInfo& PlayerVoiceInfo)
{
	if (UniqueId.IsEmpty() || PlayerVoiceInfo.PlayerName.IsEmpty()) return;
	
	PlayerId = UniqueId;
	
	if (Text_PlayerName)
	{
		Text_PlayerName->SetText(FText::FromString(PlayerVoiceInfo.PlayerName));
	}
	
	if (UPTWVoiceChatSubsystem* VoiceSubsystem = UPTWVoiceChatSubsystem::Get(this))
	{
		float CurrentVol = VoiceSubsystem->GetPlayerVoiceVolume(PlayerId);
		
		if (Slider_VoiceVolume)
		{
			Slider_VoiceVolume->SetValue(CurrentVol);
		}
		
		if (ET_VoiceVolume) 
		{
			ET_VoiceVolume->SetText(FormatFloatToText(CurrentVol));
		}
	}
}

void UPTWVoiceVolume::NativeConstruct()
{
	Super::NativeConstruct();
	
	Super::NativeConstruct();

	if (Slider_VoiceVolume)
	{
		Slider_VoiceVolume->OnValueChanged.AddDynamic(this, &UPTWVoiceVolume::OnVolumeChanged);
	}
	if (ET_VoiceVolume)
	{
		ET_VoiceVolume->OnTextCommitted.AddDynamic(this, &UPTWVoiceVolume::OnVolumeTextCommitted);
	}
}

void UPTWVoiceVolume::OnVolumeChanged(float Value)
{
	if (ET_VoiceVolume)
	{
		ET_VoiceVolume->SetText(FormatFloatToText(Value));
	}

	// 로컬 플레이어 컨트롤러에 볼륨 저장
	if (UPTWVoiceChatSubsystem* VoiceSubsystem = UPTWVoiceChatSubsystem::Get(this))
	{
		VoiceSubsystem->SetPlayerVoiceVolume(PlayerId, Value);
	}
}

void UPTWVoiceVolume::OnVolumeTextCommitted(const FText& Text, ETextCommit::Type CommitMethod)
{
	FString StringValue = Text.ToString();
	float NewValue = FCString::Atof(*StringValue);
	
	NewValue = FMath::Clamp(NewValue, 0.0f, 2.0f);

	if (Slider_VoiceVolume) Slider_VoiceVolume->SetValue(NewValue);
	if (ET_VoiceVolume) ET_VoiceVolume->SetText(FormatFloatToText(NewValue));
	
	if (UPTWVoiceChatSubsystem* VoiceSubsystem = UPTWVoiceChatSubsystem::Get(this))
	{
		VoiceSubsystem->SetPlayerVoiceVolume(PlayerId, NewValue);
	}
}

FText UPTWVoiceVolume::FormatFloatToText(float Value) const
{
	FNumberFormattingOptions NumberOptions = FNumberFormattingOptions::DefaultNoGrouping();
	NumberOptions.SetMaximumFractionalDigits(1);
	NumberOptions.SetMinimumFractionalDigits(1);

	return FText::AsNumber(Value, &NumberOptions);
}
