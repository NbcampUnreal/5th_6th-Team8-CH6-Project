// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/MiniGame/GhostChase/PTWTargetViewWidget.h"
#include "Components/Image.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "Engine/TextureRenderTarget2D.h"

void UPTWTargetViewWidget::SetRenderTarget(UTextureRenderTarget2D* RenderTarget)
{
	if (!POVImage || !BaseMaterial || !RenderTarget) return;

	// 다이나믹 머티리얼이 없다면 생성하고 이미지에 세팅
	if (!DynamicMaterial)
	{
		DynamicMaterial = UMaterialInstanceDynamic::Create(BaseMaterial, this);
		POVImage->SetBrushFromMaterial(DynamicMaterial);
	}

	// 머티리얼 내부의 Texture Parameter 에 렌더 타겟 연결
	DynamicMaterial->SetTextureParameterValue(FName("TargetTexture"), RenderTarget);
}
