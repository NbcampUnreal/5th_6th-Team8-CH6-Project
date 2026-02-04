// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "PTWGameStartTimer.generated.h"

class UTextBlock;
class APTWGameState;
/**
 * 
 */
UCLASS()
class PTW_API UPTWGameStartTimer : public UUserWidget
{
	GENERATED_BODY()
	
protected:
	virtual void NativeConstruct() override;
	virtual void NativeDestruct() override;

	/* 남은 시간 변경 콜백 */
	UFUNCTION()
	void HandleCountDownChanged(int32 CurrentCountdown);

	/* 남은 시간 표시 텍스트 */
	UPROPERTY(meta = (BindWidget))
	UTextBlock* CountdownText;
	/* GameState */
	UPROPERTY()
	APTWGameState* PTWGameState;

};
