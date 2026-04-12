#include "PTWVOIPTalker.h"

#include "System/PTWVoiceChatSubsystem.h"


void UPTWVOIPTalker::BPOnTalkingBegin_Implementation(UAudioComponent* AudioComponent)
{
	Super::BPOnTalkingBegin_Implementation(AudioComponent);
	
	if (UPTWVoiceChatSubsystem* VoiceChatSubsystem = UPTWVoiceChatSubsystem::Get(this))
	{
		if (APawn* OwnerPawn = Cast<APawn>(GetOwner()))
		{
			if (APlayerState* PS = OwnerPawn->GetPlayerState())
			{
				FString TargetPlayerId = PS->GetUniqueId().ToString();
				if (FPTWPlayerVoiceInfo* PlayerVoiceInfo = VoiceChatSubsystem->PlayerVoiceInfoList.Find(TargetPlayerId))
				{
					CurrentVolume = PlayerVoiceInfo->Volume; 
				}
			}
		}
		if (AudioComponent)
		{
			// 말하는 도중 볼륨 변경을 위해 캐싱
			CurrentAudioComponent = AudioComponent;
		
			if (VoiceSoundClass)
			{
				CurrentAudioComponent->SoundClassOverride = VoiceSoundClass;
			}
		
			// 오디오가 생성되자마자 이전에 설정해둔 개별 볼륨값으로 덮어씌웁니다.
			CurrentAudioComponent->SetVolumeMultiplier(CurrentVolume);
		}
	}
}

void UPTWVOIPTalker::SetVolume(float NewVolume)
{
	CurrentVolume = NewVolume;

	// 만약 플레이어가 현재 떠들고 있는 중이라면 즉각적으로 볼륨을 조절합니다.
	if (CurrentAudioComponent && CurrentAudioComponent->IsPlaying())
	{
		CurrentAudioComponent->SetVolumeMultiplier(CurrentVolume);
	}
}


