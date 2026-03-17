// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "PTWDeliveryHUD.generated.h"

class UPTWRankingWidget;
class UAbilitySystemComponent;
class UPTWCountDownWidget;
class UPTWBatterLevelWidget;
/**
 * 
 */
UCLASS()
class PTW_API UPTWDeliveryHUD : public UUserWidget
{
	GENERATED_BODY()
	
public:
	void InitBatterLevelWidget(UAbilitySystemComponent* ASC);
	void InitCountDownWidget();
	void UpdateCountDownWidgetCount(int32 Count);
	void UpdateRank(int32 CurRank, int32 Total);
	
public:
	UPROPERTY(meta=(BindWidget))
	TObjectPtr<UPTWBatterLevelWidget> BatterLevelWidget;
	
	UPROPERTY(meta=(BindWidget))
	TObjectPtr<UPTWCountDownWidget> CountDownWidget;
	
	UPROPERTY(meta=(BindWidget))
	TObjectPtr<UPTWRankingWidget> RankingWidget;
};
