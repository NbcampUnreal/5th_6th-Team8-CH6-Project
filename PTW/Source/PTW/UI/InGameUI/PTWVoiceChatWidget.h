// Fill out your copyright notice in the Description page of Project Settings.

#pragma once
#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "PTWVoiceChatWidget.generated.h"

class UImage;
class UTextBlock;
UCLASS()
class PTW_API UPTWVoiceChatWidget : public UUserWidget
{
	GENERATED_BODY()
	
protected:
	virtual void NativeConstruct() override;
	virtual void NativeDestruct() override;
	
public:
	void SetupWidget(FString InTalkerName);
	FString GetTalkerName() const { return TalkerName; };

protected:
	UPROPERTY(meta=(BindWidget))
	TObjectPtr<UImage> VoiceIconImage;
	
	UPROPERTY(meta=(BindWidget))
	TObjectPtr<UTextBlock> TalkerNameText;
	
	FString TalkerName;
};
