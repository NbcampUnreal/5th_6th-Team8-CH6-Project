// Fill out your copyright notice in the Description page of Project Settings.


#include "PTWVoiceChatSubsystem.h"
#include "OnlineSubsystem.h"
#include "Interfaces/VoiceInterface.h"

UPTWVoiceChatSubsystem* UPTWVoiceChatSubsystem::Get(const UObject* WorldContextObject)
{
	UWorld* World = GEngine->GetWorldFromContextObject(WorldContextObject, EGetWorldErrorMode::LogAndReturnNull);
	if (IsValid(World))
	{
		if (APlayerController* PC = World->GetFirstPlayerController())
		{
			if (ULocalPlayer* LocalPlayer = PC->GetLocalPlayer())
			{
				return LocalPlayer->GetSubsystem<UPTWVoiceChatSubsystem>();
			}
		}
	}
	return nullptr;
}

float UPTWVoiceChatSubsystem::GetIndividualVoiceVolume(const FString& PlayerID) const
{
	if (const float* FoundVolume = IndividualVoiceVolumes.Find(PlayerID))
	{
		return *FoundVolume;
	}
	return 1.0f;
}

void UPTWVoiceChatSubsystem::SetIndividualVoiceVolume(const FString& PlayerID, float NewVolume)
{
	IndividualVoiceVolumes.Add(PlayerID, NewVolume);
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
	
	Super::Deinitialize();
}

void UPTWVoiceChatSubsystem::HandlePlayerVoiceStateChanged(TSharedRef<const FUniqueNetId> TalkerId, bool bIsTalking)
{
	FString TalkerIdString = TalkerId->ToString();
	
	OnVoiceStateUpdated.Broadcast(TalkerIdString, bIsTalking);
}
