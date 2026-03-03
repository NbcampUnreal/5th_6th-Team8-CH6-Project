// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "PTWNotificationWidget.generated.h"

class UTextBlock;

UENUM(BlueprintType)
enum class ENotificationPriority : uint8
{
	Low,
	Normal,
	High,
	Critical
};

USTRUCT(BlueprintType)
struct FNotificationData
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly)
	FText Message;

	UPROPERTY(BlueprintReadOnly)
	ENotificationPriority Priority = ENotificationPriority::Normal;

	UPROPERTY(BlueprintReadOnly)
	float Duration = 2.f;

	UPROPERTY(BlueprintReadOnly)
	bool bInterrupt = false;
};

DECLARE_MULTICAST_DELEGATE(FOnNotificationFinished);
/**
 * 
 */
UCLASS()
class PTW_API UPTWNotificationWidget : public UUserWidget
{
	GENERATED_BODY()
	
public:
	void PlayMessage(const FNotificationData& Data);
	void ForceHide();

	FOnNotificationFinished OnMessageFinished;

protected:
	virtual void NativeConstruct() override;

	void ApplyStyle(ENotificationPriority InPriority);
	void HandleFinish();

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> MessageText;

protected:
	FTimerHandle MessageTimerHandle;
};
