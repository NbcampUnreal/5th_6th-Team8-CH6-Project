#include "UI/OptionsWidget/PTWPlayerVoiceVolume.h"
#include "Components/EditableText.h"
#include "Components/Slider.h"
#include "Components/TextBlock.h"
#include "CoreFramework/PTWPlayerState.h"
#include "System/PTWVoiceChatSubsystem.h"

void UPTWPlayerVoiceVolume::InitWidget(APTWPlayerState* TargetPlayerState)
{
	if (!TargetPlayerState) return;
	
	if (TargetPlayerState->GetUniqueId().IsValid())
	{
		PlayerId = TargetPlayerState->GetUniqueId()->ToString();
	}
	
	if (Text_PlayerName)
	{
		Text_PlayerName->SetText(FText::FromString(TargetPlayerState->GetPlayerName()));
	}
	
	if (UPTWVoiceChatSubsystem* VoiceSubsystem = UPTWVoiceChatSubsystem::Get(this))
	{
		float CurrentVol = VoiceSubsystem->GetIndividualVoiceVolume(PlayerId);
		
		if (Slider_VoiceVolume) Slider_VoiceVolume->SetValue(CurrentVol);
		if (ET_VoiceVolume) ET_VoiceVolume->SetText(FormatFloatToText(CurrentVol));
	}
}

void UPTWPlayerVoiceVolume::NativeConstruct()
{
	Super::NativeConstruct();
	
	Super::NativeConstruct();

	if (Slider_VoiceVolume)
	{
		Slider_VoiceVolume->OnValueChanged.AddDynamic(this, &UPTWPlayerVoiceVolume::OnVolumeChanged);
	}
	if (ET_VoiceVolume)
	{
		ET_VoiceVolume->OnTextCommitted.AddDynamic(this, &UPTWPlayerVoiceVolume::OnVolumeTextCommitted);
	}
}

void UPTWPlayerVoiceVolume::OnVolumeChanged(float Value)
{
	if (ET_VoiceVolume)
	{
		ET_VoiceVolume->SetText(FormatFloatToText(Value));
	}

	// 로컬 플레이어 컨트롤러에 볼륨 저장
	if (UPTWVoiceChatSubsystem* VoiceSubsystem = UPTWVoiceChatSubsystem::Get(this))
	{
		VoiceSubsystem->SetIndividualVoiceVolume(PlayerId, Value);
	}
}

void UPTWPlayerVoiceVolume::OnVolumeTextCommitted(const FText& Text, ETextCommit::Type CommitMethod)
{
	FString StringValue = Text.ToString();
	float NewValue = FCString::Atof(*StringValue);
	
	NewValue = FMath::Clamp(NewValue, 0.0f, 1.0f);

	if (Slider_VoiceVolume) Slider_VoiceVolume->SetValue(NewValue);
	if (ET_VoiceVolume) ET_VoiceVolume->SetText(FormatFloatToText(NewValue));
	
	if (UPTWVoiceChatSubsystem* VoiceSubsystem = UPTWVoiceChatSubsystem::Get(this))
	{
		VoiceSubsystem->SetIndividualVoiceVolume(PlayerId, NewValue);
	}
}

FText UPTWPlayerVoiceVolume::FormatFloatToText(float Value) const
{
	FNumberFormattingOptions NumberOptions = FNumberFormattingOptions::DefaultNoGrouping();
	NumberOptions.SetMaximumFractionalDigits(1);
	NumberOptions.SetMinimumFractionalDigits(1);

	return FText::AsNumber(Value, &NumberOptions);
}
