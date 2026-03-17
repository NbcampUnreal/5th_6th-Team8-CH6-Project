// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Components/TextBlock.h"
#include "PTWRankingWidget.generated.h"

/**
 * 
 */
UCLASS()
class PTW_API UPTWRankingWidget : public UUserWidget
{
	GENERATED_BODY()
	
public:
	virtual void NativeConstruct() override;
	void UpdateRank(int32 NewRank, int32 Total);
	
protected:
	UPROPERTY(EditAnywhere, meta=(BindWidget))
	TObjectPtr<UTextBlock> RankingTextBlock;
};
