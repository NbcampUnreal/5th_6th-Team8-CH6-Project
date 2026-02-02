// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "GameplayTagContainer.h"
#include "PTWCrosshair.generated.h"

class UAbilitySystemComponent;
/**
 * 
 */
UCLASS()
class PTW_API UPTWCrosshair : public UUserWidget
{
	GENERATED_BODY()
	
public:
	// 초기화 시 ASC를 전달받음
	void InitWithASC(UAbilitySystemComponent* InASC);
	
protected:
	virtual void NativeConstruct() override;
	virtual void NativeDestruct() override;

	// 태그 변경 시 호출될 콜백
	void OnStateTagChanged(const FGameplayTag Tag, int32 NewCount);
	void UpdateVisibility();

	UFUNCTION()
	void SetCrosshairVisibility(bool bVisible);

	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional))
	TObjectPtr<class UImage> CrosshairImage;

private:
	UPROPERTY()
	TObjectPtr<UAbilitySystemComponent> CachedASC;

	FGameplayTag EquipTag;
	FGameplayTag SprintTag;

	FDelegateHandle EquipTagHandle;
	FDelegateHandle SprintTagHandle;
};
