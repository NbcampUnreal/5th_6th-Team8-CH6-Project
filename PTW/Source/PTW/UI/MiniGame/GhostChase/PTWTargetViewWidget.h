// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "PTWTargetViewWidget.generated.h"

class UImage;
/**
 * 
 */
UCLASS()
class PTW_API UPTWTargetViewWidget : public UUserWidget
{
	GENERATED_BODY()
	
public:
	/** Controller에서 생성된 RenderTarget을 이미지 슬롯에 바인딩 */
	void SetRenderTarget(UTextureRenderTarget2D* RenderTarget);

protected:
	/** UI에서 렌더링을 담당할 Image 컴포넌트 */
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UImage> POVImage;

	/** 동적으로 텍스처를 교체하기 위한 다이나믹 머티리얼 인스턴스 */
	UPROPERTY()
	TObjectPtr<UMaterialInstanceDynamic> DynamicMaterial;

	/** 기본이 되는 부모 머티리얼 (렌더 타겟 파라미터가 포함된 것) */
	UPROPERTY(EditDefaultsOnly, Category = "UI")
	TObjectPtr<UMaterialInterface> BaseMaterial;
};
