// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "AbilitySystemComponent.h" // FOnAttributeChangeData, FDelegateHandle 사용을 위해 필요
#include "PTWHealthBar.generated.h"

class UProgressBar;
class UAbilitySystemComponent;
/**
 * 
 */
UCLASS()
class PTW_API UPTWHealthBar : public UUserWidget
{
	GENERATED_BODY()
	
public:
	virtual void NativeConstruct() override;
	virtual void NativeDestruct() override;

	/* ASC를 전달받고 초기화 */
	void InitWithASC(UAbilitySystemComponent* ASC);

protected:
	/* 체력바 */
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UProgressBar> HealthBar;

private:
	/* 델리게이트 연결 */
	void BindGASDelegates(UAbilitySystemComponent* ASC);
	/* 델리게이트 : 현재체력 변경 감지 */
	void OnHealthAttributeChanged(const FOnAttributeChangeData& Data);
	/* 델리게이트 : 최대체력 변경 감지 */
	void OnMaxHealthAttributeChanged(const FOnAttributeChangeData& Data);
	/* 체력바 갱신 */
	void UpdateHealthBar(float CurrentHealth, float MaxHealth);

	/* Ability System Component (UI가 직접 찾지 않음, InitWithASC를 통해 전달받음) */
	UPROPERTY()
	TObjectPtr<UAbilitySystemComponent> AbilitySystemComponent;

	/* Delegate Handles (unbind 필요) */
	FDelegateHandle HealthChangedHandle;
	FDelegateHandle MaxHealthChangedHandle;

};
