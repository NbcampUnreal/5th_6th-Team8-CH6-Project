// Fill out your copyright notice in the Description page of Project Settings.


#include "PTWVoiceChatSubsystem.h"
#include "OnlineSubsystem.h"
#include "Interfaces/VoiceInterface.h"

void UPTWVoiceChatSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);
	
	// FTimerHandle TimerHandle;
	// GetWorld()->GetTimerManager().SetTimer(TimerHandle, [=, this]()
	// {
		if (IOnlineSubsystem* OnlineSubsystem = IOnlineSubsystem::Get())
		{
			if (IOnlineVoicePtr VoiceInterface = OnlineSubsystem->GetVoiceInterface())
			{
				VoiceStateDelegateHandle = VoiceInterface->AddOnPlayerTalkingStateChangedDelegate_Handle(
					FOnPlayerTalkingStateChangedDelegate::CreateUObject(this, &ThisClass::HandlePlayerVoiceStateChanged));
			}
		}
	// }, 15.0f, false);
	
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
	UE_LOG(LogTemp, Error, TEXT("HandlePlayerVoiceStateChanged"));
	FString TalkerIdString = TalkerId->ToString();
	
	OnVoiceStateUpdated.Broadcast(TalkerIdString, bIsTalking);
}
