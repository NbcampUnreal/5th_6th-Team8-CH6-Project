// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UI/LoadingScreen/PTWLoadingWidgetBase.h"
#include "PTWLoadingMiniGame.generated.h"

/**
 * 
 */
UCLASS()
class PTW_API UPTWLoadingMiniGame : public UPTWLoadingWidgetBase
{
	GENERATED_BODY()

public:
	/** 미니게임 전용 정보 세팅 */
	void SetupMiniGameInfo(const FText& InName, const FText& InDesc);

protected:
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> MapNameText;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> DescriptionText;
};
