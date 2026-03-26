// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "PTWSpectatorHUD.generated.h"

class UBorder;
class UTextBlock;

/**
 * 
 */
UCLASS()
class PTW_API UPTWSpectatorHUD : public UUserWidget
{
	GENERATED_BODY()
public:
	UFUNCTION(BlueprintCallable)
	void SetSpectateTargetName(const FString& NewName);
	void HandleNativeVisibilityChanged(ESlateVisibility InVisibility);
	
protected:
	virtual void NativeConstruct() override;
	virtual void NativeDestruct() override;
public:
protected:
	UPROPERTY(meta=(BindWidget))
	TObjectPtr<UBorder> SpectateTargetBorder;
	
	UPROPERTY(meta=(BindWidget))
	TObjectPtr<UTextBlock> SpectateTargetText;
private:
	
};
