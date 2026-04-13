#include "PTWVoiceChatWidget.h"

#include "Components/Image.h"
#include "Components/TextBlock.h"
#include "CoreFramework/PTWPlayerController.h"

void UPTWVoiceChatWidget::NativeConstruct()
{
	Super::NativeConstruct();
}

void UPTWVoiceChatWidget::NativeDestruct()
{
	Super::NativeDestruct();
}

void UPTWVoiceChatWidget::SetupWidget(FString InTalkerName)
{
	TalkerName = InTalkerName;
	TalkerNameText->SetText(FText::FromString(TalkerName));
}

void UPTWVoiceChatWidget::SetEnabledVoiceIcon()
{
	if (IsValid(EnabledVoiceIcon))
	{
		CurrentVoiceIcon->SetBrushFromTexture(EnabledVoiceIcon);
	}
}

void UPTWVoiceChatWidget::SetTalkingVoiceIcon()
{
	if (IsValid(EnabledVoiceIcon))
	{
		CurrentVoiceIcon->SetBrushFromTexture(TalkingVoiceIcon);
	}
}
