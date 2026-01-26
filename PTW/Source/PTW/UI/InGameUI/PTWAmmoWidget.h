// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "PTWAmmoWidget.generated.h"

class UTextBlock;
/**
 * 
 */
UCLASS()
class PTW_API UPTWAmmoWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	UFUNCTION()
	void UpdateAmmoWidget(int32 CurrentAmmo, int32 ReserveAmmo);

protected:
	virtual void NativeConstruct() override;
	virtual void NativeDestruct() override;

	// 현재 탄창 내 탄약 
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> CurrentAmmoText;

	// 보유 중인 전체 여분 탄약 
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> ReserveAmmoText;
};
