// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/InGameUI/PTWItemSlot.h"
#include "Components/Image.h"
#include "Inventory/PTWItemDefinition.h"
#include "Engine/Texture2D.h"

void UPTWItemSlot::SetupSlot(const UPTWItemDefinition* ItemDef)
{
	/* 데이터 에셋과 위젯의 이미지 컴포넌트 유효성 검사 */
	if (!ItemDef || !ItemIcon) return;

	/* 소프트 포인터를 로드하여 실제 텍스처 포인터로 변환 */
	UTexture2D* LoadedTexture = ItemDef->ItemIcon.LoadSynchronous();

	if (LoadedTexture)
	{
		ItemIcon->SetBrushFromTexture(LoadedTexture);
	}

	UE_LOG(LogTemp, Warning, TEXT("SetupSlot"));
}
