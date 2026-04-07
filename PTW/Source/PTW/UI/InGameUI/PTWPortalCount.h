// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "PTWPortalCount.generated.h"

class UTextBlock;
class APTWGameState;
/**
 * 
 */
UCLASS()
class PTW_API UPTWPortalCount : public UUserWidget
{
	GENERATED_BODY()

public:
	virtual void NativeDestruct() override;

	void InitializeGameState();

	UFUNCTION()
	void UpdatePortalText(int32 Current, int32 Required);

protected:
	UPROPERTY(meta = (BindWidget))
	UTextBlock* PortalCountText;

private:
	UPROPERTY()
	APTWGameState* PTWGameState;

	FTimerHandle PortalCount_InitGameState;

};
