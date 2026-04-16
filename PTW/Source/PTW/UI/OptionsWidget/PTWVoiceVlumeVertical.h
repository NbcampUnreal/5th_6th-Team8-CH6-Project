#pragma once

#include "CoreMinimal.h"
#include "Components/VerticalBox.h"
#include "PTWVoiceVlumeVertical.generated.h"

class UPTWVoiceVolume;
/**
 * 플레이어 보이스볼륨 위젯을 보관하는 버티컬 박스
 */
UCLASS()
class PTW_API UPTWVoiceVlumeVertical : public UVerticalBox
{
	GENERATED_BODY()
	
public:
	void InitializeWidget();
	
	UFUNCTION()
	void HandleVoiceChatDisconnected();
protected:
	virtual TSharedRef<SWidget> RebuildWidget() override;
	virtual void ReleaseSlateResources(bool bReleaseChildren) override;
	// 보이스챗 플레이어 리스트 위젯
	UPROPERTY(EditDefaultsOnly)
	TSubclassOf<UPTWVoiceVolume> VoiceVolumeClass;
	
	UPROPERTY()
	TMap<FString, UPTWVoiceVolume*> PlayerVoiceVolumes;
};
