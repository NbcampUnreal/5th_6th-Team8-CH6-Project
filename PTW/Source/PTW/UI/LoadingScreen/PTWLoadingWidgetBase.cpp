// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/LoadingScreen/PTWLoadingWidgetBase.h"
#include "Components/Image.h"
#include "Components/TextBlock.h"
#include "CoreFramework/Game/GameState/PTWGameState.h"

void UPTWLoadingWidgetBase::NativeConstruct()
{
	Super::NativeConstruct();

	//// GameState의 인원수 변경 델리게이트 바인딩
	//if (APTWGameState* GS = GetWorld()->GetGameState<APTWGameState>())
	//{
	//	GS->OnPortalCountChanged.AddDynamic(this, &UPTWLoadingWidgetBase::OnPlayerCountChanged);
	//	// 초기값 세팅
	//	OnPlayerCountChanged(GS->LoadedPlayerCount, GS->TotalPlayerCount);
	//}
}

void UPTWLoadingWidgetBase::NativeDestruct()
{
	/*if (APTWGameState* GS = GetWorld()->GetGameState<APTWGameState>())
	{
		GS->OnPortalCountChanged.RemoveDynamic(this, &UPTWLoadingWidgetBase::OnPlayerCountChanged);
	}*/

	Super::NativeDestruct();
}

void UPTWLoadingWidgetBase::InitBaseUI(TSoftObjectPtr<UTexture2D> Background)
{
	if (LoadingBackgroundImage && !Background.IsNull())
	{
		LoadingBackgroundImage->SetBrushFromSoftTexture(Background);
	}
}

//void UPTWLoadingWidgetBase::OnPlayerCountChanged(int32 Current, int32 Total)
//{
//	if (PlayerCountText)
//	{
//		// 형식: ( 1 / 8 )
//		FString CountStr = FString::Printf(TEXT("( % d / % d )"), Current, Total);
//		PlayerCountText->SetText(FText::FromString(CountStr));
//	}
//}
