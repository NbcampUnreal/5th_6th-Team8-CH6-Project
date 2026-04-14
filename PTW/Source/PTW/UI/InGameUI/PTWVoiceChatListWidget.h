#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "PTWVoiceChatListWidget.generated.h"

class UVerticalBox;
class UPTWVoiceChatWidget;
/**
 * 플레이어 VoiceChat 위젯들을 보관하는 위젯
 */
UCLASS()
class PTW_API UPTWVoiceChatListWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	void Init();
	UFUNCTION(BlueprintCallable, Category = "Voice Chat")
	void OnVoiceStateChanged(const FString& PlayerNetId, bool bIsTalking);
	
	FString GetPlayerNameFromNetId(const FString& TargetNetId);
	
	UFUNCTION()
	void HandleChangedVoiceChatState(bool bIsActive);
	
protected:
	virtual void NativeConstruct() override;
	virtual void NativeDestruct() override;

	UFUNCTION()
	void HandlePlayerConnected(const FString& UniqueId);
	UFUNCTION()
	void HandlePlayerDisconnected(const FString& UniqueId);
	UFUNCTION()
	void HandleAllPlayersConnected(const FString& UniqueId);
protected:
	UPROPERTY()
	TMap<FString, UPTWVoiceChatWidget*> PlayerVoiceChats;
	
	UPROPERTY(meta=(BindWidget))
	TObjectPtr<UVerticalBox> VoiceChatList;
	
	UPROPERTY(EditDefaultsOnly)
	TSubclassOf<UPTWVoiceChatWidget> VoiceChatWidgetClass;
};
