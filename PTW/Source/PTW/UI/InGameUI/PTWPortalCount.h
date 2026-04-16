// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "GameplayTagContainer.h"
#include "PTWPortalCount.generated.h"

class UTextBlock;
class APTWGameState;
class UAbilitySystemComponent;

/**
 * 
 */
UCLASS()
class PTW_API UPTWPortalCount : public UUserWidget
{
	GENERATED_BODY()

public:
	virtual void NativeDestruct() override;

	void InitWithASC(UAbilitySystemComponent* AbilitySystemComponent);

	void InitializeGameState();

	UFUNCTION()
	void UpdatePortalText(int32 Current, int32 Required);

protected:
	void OnPortalTagChanged(const FGameplayTag Tag, int32 NewCount);

	UPROPERTY(meta = (BindWidget))
	UTextBlock* PortalCountText;

private:
	UPROPERTY()
	TObjectPtr<UAbilitySystemComponent> ASC;

	UPROPERTY()
	APTWGameState* PTWGameState;

	FTimerHandle PortalCount_InitGameState;

};
