// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
// #include "Subsystems/GameInstanceSubsystem.h"
#include "Subsystems/LocalPlayerSubsystem.h"
#include "PTWVoiceChatSubsystem.generated.h"

/**
 * 
 */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnVoiceChatStateUpdated, const FString&, PlayerNetId, bool, bIsTalking);

class FUniqueNetId;
UCLASS()
class PTW_API UPTWVoiceChatSubsystem : public ULocalPlayerSubsystem
{
	GENERATED_BODY()
	
public:
	UPROPERTY(BlueprintAssignable, Category = "Voice Chat")
	FOnVoiceChatStateUpdated OnVoiceStateUpdated;
protected:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;
	
	void HandlePlayerVoiceStateChanged(TSharedRef<const FUniqueNetId> TalkerId, bool bIsTalking);

	FDelegateHandle VoiceStateDelegateHandle;
};
