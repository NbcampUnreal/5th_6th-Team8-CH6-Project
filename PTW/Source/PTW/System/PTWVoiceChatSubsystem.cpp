#include "PTWVoiceChatSubsystem.h"
#include "OnlineSubsystem.h"
#include "CoreFramework/Game/GameState/PTWGameState.h"
#include "GameFramework/PlayerState.h"
#include "Interfaces/VoiceInterface.h"

UPTWVoiceChatSubsystem* UPTWVoiceChatSubsystem::Get(const UObject* WorldContextObject)
{
	if (GEngine)
	{
		if (UWorld* World = GEngine->GetWorldFromContextObject(WorldContextObject, EGetWorldErrorMode::LogAndReturnNull))
		{
			if (APlayerController* PC = World->GetFirstPlayerController())
			{
				if (ULocalPlayer* LocalPlayer = PC->GetLocalPlayer())
				{
					return LocalPlayer->GetSubsystem<UPTWVoiceChatSubsystem>();
				}
			}
		}
	}
	return nullptr;
}

float UPTWVoiceChatSubsystem::GetPlayerVoiceVolume(const FString& PlayerID)
{
	if (PlayerVoiceInfoList.Contains(PlayerID))
	{
		return PlayerVoiceInfoList[PlayerID].Volume;
	}
	else
	{
		PlayerVoiceInfoList.Add(PlayerID, FPTWPlayerVoiceInfo());
		return PlayerVoiceInfoList[PlayerID].Volume;
	}
}

void UPTWVoiceChatSubsystem::SetPlayerVoiceVolume(const FString& PlayerID, float NewVolume)
{
	if (PlayerVoiceInfoList.Find(PlayerID))
	{
		PlayerVoiceInfoList[PlayerID].Volume = NewVolume;
	}
	else
	{
		PlayerVoiceInfoList.Add(PlayerID, FPTWPlayerVoiceInfo());
		PlayerVoiceInfoList[PlayerID].Volume = NewVolume;
	}
}

void UPTWVoiceChatSubsystem::OnPlayerStateChanged(APlayerState* PlayerState, bool bIsAdded)
{
	if (!PlayerState) return;
	if (bIsAdded)
	{
		FString UniqueId = PlayerState->GetUniqueId().ToString();
		if (UniqueId.IsEmpty()) return;
		
		if (PlayerVoiceInfoList.Contains(UniqueId))
		{
			PlayerVoiceInfoList[UniqueId] = FPTWPlayerVoiceInfo();
		}
		else
		{
			PlayerVoiceInfoList.Add(UniqueId, FPTWPlayerVoiceInfo());
		}
	}
	else
	{
		FString UniqueId = PlayerState->GetUniqueId().ToString();
		if (PlayerVoiceInfoList.Contains(UniqueId))
		{
			PlayerVoiceInfoList.Remove(UniqueId);
		}
	}
}


void UPTWVoiceChatSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);

	if (IOnlineSubsystem* OnlineSubsystem = IOnlineSubsystem::Get())
	{
		if (IOnlineVoicePtr VoiceInterface = OnlineSubsystem->GetVoiceInterface())
		{
			VoiceStateDelegateHandle = VoiceInterface->AddOnPlayerTalkingStateChangedDelegate_Handle(
				FOnPlayerTalkingStateChangedDelegate::CreateUObject(this, &ThisClass::HandlePlayerVoiceStateChanged));
		}
	}
}

void UPTWVoiceChatSubsystem::Deinitialize()
{
	if (IOnlineSubsystem* OnlineSubsystem = IOnlineSubsystem::Get())
	{
		if (IOnlineVoicePtr VoiceInterface = OnlineSubsystem->GetVoiceInterface())
		{
			VoiceInterface->ClearOnPlayerTalkingStateChangedDelegate_Handle(VoiceStateDelegateHandle);
		}
	}
	
	PlayerVoiceInfoList.Empty();
	Super::Deinitialize();
}

void UPTWVoiceChatSubsystem::HandlePlayerVoiceStateChanged(TSharedRef<const FUniqueNetId> TalkerId, bool bIsTalking)
{
	FString TalkerIdString = TalkerId->ToString();
	
	OnVoiceStateUpdated.Broadcast(TalkerIdString, bIsTalking);
}
