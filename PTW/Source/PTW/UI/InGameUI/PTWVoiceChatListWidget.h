// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "PTWVoiceChatListWidget.generated.h"

class UVerticalBox;
class UPTWVoiceChatWidget;
/**
 * 
 */
UCLASS()
class PTW_API UPTWVoiceChatListWidget : public UUserWidget
{
	GENERATED_BODY()
	
protected:
	virtual void NativeConstruct() override;
	virtual void NativeDestruct() override;
public:
	void Init();
	UFUNCTION(BlueprintCallable, Category = "Voice Chat")
	void OnVoiceStateChanged(const FString& PlayerNetId, bool bIsTalking);
	
	FString GetPlayerNameFromNetId(const FString& TargetNetId);
protected:
	UPROPERTY(meta=(BindWidget))
	TObjectPtr<UVerticalBox> VoiceChatListVerticalBox;
	
	UPROPERTY(EditDefaultsOnly)
	TSubclassOf<UPTWVoiceChatWidget> VoiceChatWidgetClass;
	
	UPROPERTY()
	TArray<UPTWVoiceChatWidget*> VoiceChatWidgets;
};
