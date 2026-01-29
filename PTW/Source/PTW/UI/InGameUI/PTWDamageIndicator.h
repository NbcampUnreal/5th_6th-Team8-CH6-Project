// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "PTWDamageIndicator.generated.h"

/**
 * 
 */
UCLASS()
class PTW_API UPTWDamageIndicator : public UUserWidget
{
	GENERATED_BODY()
	
public:
	/** 초기화 */
	void Init(const FVector& DamageCauserLocation);

protected:
	virtual void NativeTick(const FGeometry& MyGeometry, float InDeltaTime) override;

private:
	/** 남은 시간 */
	float RemainingLifeTime = 0.f;

	/** 전체 수명 */
	UPROPERTY(EditDefaultsOnly, Category = "DamageIndicator")
	float LifeTime = 0.8f;

	/** Fade 시작 시점 */
	UPROPERTY(EditDefaultsOnly, Category = "DamageIndicator")
	float FadeStartTime = 0.3f;

	/** 방향 회전 */
	void UpdateRotation(const FVector& DamageCauserLocation);

	/** 투명도 업데이트 */
	void UpdateOpacity();
};
