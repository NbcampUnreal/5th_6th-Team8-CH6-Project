#include "PTWVoiceChatSubsystem.h"
#include "OnlineSubsystem.h"
#include "CoreFramework/Game/GameInstance/PTWGameInstance.h"
#include "GameFramework/GameStateBase.h"
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
	if (UPTWGameInstance* GI = GetWorld()->GetGameInstance<UPTWGameInstance>())
	{
		GI->OnPlayerConnected.AddUniqueDynamic(this, &ThisClass::HandlePlayerConnected);
		GI->OnPlayerDisconnected.AddUniqueDynamic(this, &ThisClass::HandlePlayerDisconnected);
		GI->OnAllPlayersDisconnected.AddUniqueDynamic(this, &ThisClass::HandleAllPlayersDisconnected);
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
	
	if (UPTWGameInstance* GI = GetWorld()->GetGameInstance<UPTWGameInstance>())
	{
		GI->OnPlayerConnected.RemoveDynamic(this, &ThisClass::HandlePlayerConnected);
		GI->OnPlayerDisconnected.RemoveDynamic(this, &ThisClass::HandlePlayerDisconnected);
		GI->OnAllPlayersDisconnected.RemoveDynamic(this, &ThisClass::HandleAllPlayersDisconnected);
	}
	PlayerVoiceInfoList.Empty();
	Super::Deinitialize();
}

void UPTWVoiceChatSubsystem::HandlePlayerConnected(const FString& UniqueId)
{
	// TODO: 세이브파일에 데이터가 저장된다면 여기서 설정하면 될 것 같음.
	FPTWPlayerVoiceInfo PlayerVoiceInfo;
	
	if (AGameStateBase* GS = GetWorld()->GetGameState())
	{
		for (APlayerState* PS : GS->PlayerArray)
		{
			if (IsValid(PS) && PS->GetUniqueId().ToString() == UniqueId)
			{
				PlayerVoiceInfo.PlayerName =  PS->GetPlayerName();
				break;
			}
		}
	}
	
	PlayerVoiceInfoList.Add(UniqueId, PlayerVoiceInfo);
	OnVoiceChatConnected.Broadcast(UniqueId);
}

void UPTWVoiceChatSubsystem::HandlePlayerDisconnected(const FString& UniqueId)
{
	OnVoiceChatDisconnected.Broadcast(UniqueId);
	PlayerVoiceInfoList.Remove(UniqueId);
}

void UPTWVoiceChatSubsystem::HandleAllPlayersDisconnected(const FString& UniqueId)
{
	OnAllVoiceChatDisconnected.Broadcast(UniqueId);
	PlayerVoiceInfoList.Reset();
}

void UPTWVoiceChatSubsystem::HandlePlayerVoiceStateChanged(TSharedRef<const FUniqueNetId> TalkerId, bool bIsTalking)
{
	FString TalkerIdString = TalkerId->ToString();
	
	OnVoiceStateUpdated.Broadcast(TalkerIdString, bIsTalking);
}
