#pragma once

#include "CoreMinimal.h"
#include "Net/VoiceConfig.h"
#include "PTWVOIPTalker.generated.h"

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class PTW_API UPTWVOIPTalker : public UVOIPTalker
{
	GENERATED_BODY()

public:
	virtual void BPOnTalkingBegin_Implementation(UAudioComponent* AudioComponent) override;
	
	UFUNCTION(BlueprintCallable, Category = "Voice")
	void SetVolume(float NewVolume);
	
protected:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Voice Options")
	TObjectPtr<USoundClass> VoiceSoundClass;
	
private:
	// 현재 재생 중인 오디오 컴포넌트를 캐싱해두기 위한 포인터
	UPROPERTY()
	TObjectPtr<UAudioComponent> CurrentAudioComponent;
    
	// 이 플레이어에게 설정된 볼륨 (기본값 1.0)
	float CurrentVolume = 1.0f;
};
