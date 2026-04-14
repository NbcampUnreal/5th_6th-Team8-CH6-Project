#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "PTWVoiceVolume.generated.h"

class APlayerState;
class UTextBlock;
class UEditableText;
class USlider;
struct FPTWPlayerVoiceInfo;
/**
 * 다른 플레이어들의 음성채팅 소리를 조절하는 위젯입니다.
 */

UCLASS()
class PTW_API UPTWVoiceVolume : public UUserWidget
{
	GENERATED_BODY()
	
public:
	void InitWidget(const FString& UniqueId, const FPTWPlayerVoiceInfo& PlayerVoiceInfo);
	
	FORCEINLINE FString GetPlayerId() const { return PlayerId; };
protected:
	virtual void NativeConstruct() override;
	
	UFUNCTION()
	void OnVolumeChanged(float Value);
	
	UFUNCTION()
	void OnVolumeTextCommitted(const FText& Text, ETextCommit::Type CommitMethod);

	FText FormatFloatToText(float Value) const;
	
protected:
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> Text_PlayerName;
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<USlider> Slider_VoiceVolume;
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UEditableText> ET_VoiceVolume;
	
	FString PlayerId;
};
