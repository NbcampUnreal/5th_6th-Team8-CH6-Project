// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "PTWLoadingWidgetBase.generated.h"

class UImage; 
class UTextBlock;
/**
 * 
 */
UCLASS()
class PTW_API UPTWLoadingWidgetBase : public UUserWidget
{
	GENERATED_BODY()
	
public:
	/** 기본 UI 설정 (이미지만 설정) */
	virtual void InitBaseUI(TSoftObjectPtr<UTexture2D> Background);

protected:
	virtual void NativeConstruct() override;
	virtual void NativeDestruct() override;

	/** 인원수 변경 시 GameState에서 호출될 함수 */
	UFUNCTION()
	virtual void OnPlayerCountChanged(int32 Current, int32 Total);

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UImage> LoadingBackgroundImage;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> PlayerCountText;
};
