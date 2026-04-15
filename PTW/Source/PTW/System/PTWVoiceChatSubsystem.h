#pragma once

#include "CoreMinimal.h"
#include "Subsystems/LocalPlayerSubsystem.h"
#include "PTWVoiceChatSubsystem.generated.h"

class FUniqueNetId;

USTRUCT(BlueprintType)
struct FPTWPlayerVoiceInfo
{
	GENERATED_BODY()
	
	UPROPERTY()
	FString PlayerName = FString("");
	
	UPROPERTY(BlueprintReadWrite)
	float Volume = 1.0f;
	
	UPROPERTY(BlueprintReadWrite)
	bool bIsMuted = false;
	
	UPROPERTY(BlueprintReadOnly, NotReplicated)
	bool bIsMicActive = false;
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnVoiceChatStateUpdated, const FString&, UniqueId, bool, bIsTalking);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnVoiceChatConnectionSignature, const FString, UniqueId);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnAllVoiceChatDisconnectedSignature);

/**
 * Steam VoiceChat을 관리하는 서브 시스템입니다.
 */
UCLASS()
class PTW_API UPTWVoiceChatSubsystem : public ULocalPlayerSubsystem
{
	GENERATED_BODY()

public:
	static UPTWVoiceChatSubsystem* Get(const UObject* WorldContextObject);
	float GetPlayerVoiceVolume(const FString& PlayerID);

	void SetPlayerVoiceVolume(const FString& PlayerID, float NewVolume);
	
	UFUNCTION()
	void OnPlayerStateChanged(APlayerState* PlayerState, bool bIsAdded);
protected:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;
	
	UFUNCTION()
	void HandlePlayerConnected(const FString& UniqueId);
	UFUNCTION()
	void HandlePlayerDisconnected(const FString& UniqueId);
	UFUNCTION()
	void HandleAllPlayersDisconnected();
	
	void HandlePlayerVoiceStateChanged(TSharedRef<const FUniqueNetId> TalkerId, bool bIsTalking);

public:
	TMap<FString, FPTWPlayerVoiceInfo> PlayerVoiceInfoList;
	
public:
	UPROPERTY(BlueprintAssignable, Category = "Voice Chat")
	FOnVoiceChatStateUpdated OnVoiceStateUpdated;
	
	FOnVoiceChatConnectionSignature OnVoiceChatConnected;
	FOnVoiceChatConnectionSignature OnVoiceChatDisconnected;
	FOnAllVoiceChatDisconnectedSignature AllVoiceChatDisconnected;
	
protected:
	FDelegateHandle VoiceStateDelegateHandle;
};
