// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/InGameUI/PTWPortalCount.h"
#include "Components/TextBlock.h"
#include "CoreFramework/Game/GameState/PTWGameState.h"

void UPTWPortalCount::NativeDestruct()
{
	PTWGameState->OnPortalCountChanged.RemoveDynamic(this, &UPTWPortalCount::UpdatePortalText);

	Super::NativeDestruct();
}

void UPTWPortalCount::InitializeGameState()
{
	PTWGameState = GetWorld() ? GetWorld()->GetGameState<APTWGameState>() : nullptr;

	if (IsValid(PTWGameState))
	{
		// 중복 바인딩 방지 
		PTWGameState->OnPortalCountChanged.RemoveDynamic(this, &UPTWPortalCount::UpdatePortalText);

		GetWorld()->GetTimerManager().ClearTimer(PortalCount_InitGameState);

		UE_LOG(LogTemp, Warning, TEXT("PTWTimer : InitTimer"));

		// 델리게이트 바인딩
		PTWGameState->OnPortalCountChanged.AddDynamic(this, &UPTWPortalCount::UpdatePortalText);

		// 초기 값 반영
		UpdatePortalText(PTWGameState->GetPortalCurrent(),PTWGameState->GetPortalRequired());
	}
	else
	{
		if (!PortalCount_InitGameState.IsValid())
		{
			GetWorld()->GetTimerManager().SetTimer(
				PortalCount_InitGameState,
				this,
				&UPTWPortalCount::InitializeGameState,
				0.1f,
				true
			);
		}
	}
}

void UPTWPortalCount::UpdatePortalText(int32 Current, int32 Required)
{
	if (PortalCountText)
	{
		// 최적화된 FText 생성 (Current / Required)
		FText FormattedText = FText::Format(
			NSLOCTEXT("UI", "PortalCountFormat", "{0} / {1}"),
			FText::AsNumber(Current),
			FText::AsNumber(Required)
		);

		PortalCountText->SetText(FormattedText);
	}
}
