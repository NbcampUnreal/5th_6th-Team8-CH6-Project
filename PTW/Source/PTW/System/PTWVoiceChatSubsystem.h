#pragma once

#include "CoreMinimal.h"
// #include "Subsystems/GameInstanceSubsystem.h"
#include "Subsystems/LocalPlayerSubsystem.h"
#include "PTWVoiceChatSubsystem.generated.h"

/**
 * Steam VoiceChat을 관리하는 서브 시스템입니다.
 */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnVoiceChatStateUpdated, const FString&, PlayerNetId, bool, bIsTalking);

class FUniqueNetId;
UCLASS()
class PTW_API UPTWVoiceChatSubsystem : public ULocalPlayerSubsystem
{
	GENERATED_BODY()

protected:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;
	
	void HandlePlayerVoiceStateChanged(TSharedRef<const FUniqueNetId> TalkerId, bool bIsTalking);

public:
	UPROPERTY(BlueprintAssignable, Category = "Voice Chat")
	FOnVoiceChatStateUpdated OnVoiceStateUpdated;
	
protected:
	FDelegateHandle VoiceStateDelegateHandle;
};
