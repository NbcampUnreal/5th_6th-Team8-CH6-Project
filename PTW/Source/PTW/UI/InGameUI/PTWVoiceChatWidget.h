#pragma once
#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "PTWVoiceChatWidget.generated.h"

class UImage;
class UTextBlock;
class UTexture2D;

/*
 * 플레이어 VoiceChat 위젯
 */
UCLASS()
class PTW_API UPTWVoiceChatWidget : public UUserWidget
{
	GENERATED_BODY()
	
protected:
	virtual void NativeConstruct() override;
	virtual void NativeDestruct() override;
	
public:
	void InitializeWidget(FString InTalkerName);
	FString GetTalkerName() const { return TalkerName; };

	void SetEnabledVoiceIcon();
	void SetTalkingVoiceIcon();

protected:
	UPROPERTY(meta=(BindWidget))
	TObjectPtr<UImage> CurrentVoiceIcon;
	
	UPROPERTY(meta=(BindWidget))
	TObjectPtr<UTextBlock> TalkerNameText;
	
	FString TalkerName;
	
	UPROPERTY(EditDefaultsOnly)
	TObjectPtr<UTexture2D> EnabledVoiceIcon;
	
	UPROPERTY(EditDefaultsOnly)
	TObjectPtr<UTexture2D> TalkingVoiceIcon;
};
