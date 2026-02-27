// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "GameplayEffectTypes.h" // FOnAttributeChangeData 정의가 포함된 헤더
#include "PTWBombWarning.generated.h"

class APTWBombActor;
class UImage;
/**
 * 
 */
UCLASS()
class PTW_API UPTWBombWarning : public UUserWidget
{
	GENERATED_BODY()
	
public:
	// PC에서 호출하여 대상 폭탄을 설정
	void SetTargetBomb(APTWBombActor* InBomb);

protected:
	virtual void NativeConstruct() override;
	virtual void NativeDestruct() override;

	// 어트리뷰트가 변경될 때 호출될 콜백 함수
	void OnBombTimeChanged(const FOnAttributeChangeData& Data);

protected:
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UImage> WarningImage;

	/* 점멸 기본 속도 */
	UPROPERTY(EditAnywhere, Category = "BombUI")
	float BaseBlinkSpeed = 1.f;

private:
	UPROPERTY()
	TObjectPtr<APTWBombActor> TargetBomb;

	/* 머터리얼 파라미터 조절을 위한 다이내믹 머터리얼 */
	UPROPERTY()
	TObjectPtr<UMaterialInstanceDynamic> WarningDynamicMat;

	/* 델리게이트 핸들 */
	FDelegateHandle TimeChangedHandle;
};
