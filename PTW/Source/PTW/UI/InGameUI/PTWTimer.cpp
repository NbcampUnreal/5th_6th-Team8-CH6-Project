// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/InGameUI/PTWTimer.h"
#include "CoreFramework/Game/GameState/PTWGameState.h"
#include "Components/TextBlock.h"
#include "Kismet/GameplayStatics.h"

void UPTWTimer::NativeConstruct()
{
	Super::NativeConstruct();

	PTWGameState = GetWorld() ? GetWorld()->GetGameState<APTWGameState>() : nullptr;

	if (PTWGameState)
	{
		// 델리게이트 바인딩
		PTWGameState->OnRemainTimeChanged.AddDynamic(
			this, &UPTWTimer::HandleRemainTimeChanged
		);

		// 초기 값 반영
		HandleRemainTimeChanged(PTWGameState->GetRemainTime());
	}
}

void UPTWTimer::NativeDestruct()
{
	if (PTWGameState)
	{
		PTWGameState->OnRemainTimeChanged.RemoveDynamic(
			this, &UPTWTimer::HandleRemainTimeChanged
		);
	}

	Super::NativeDestruct();
}

void UPTWTimer::HandleRemainTimeChanged(int32 NewRemainTime)
{
	if (!RemainTimeText) return;

	RemainTimeText->SetText(FormatTime(NewRemainTime));
}

FText UPTWTimer::FormatTime(int32 Seconds) const
{
	const int32 Min = Seconds / 60;
	const int32 Sec = Seconds % 60;

	return FText::FromString(
		FString::Printf(TEXT("%02d:%02d"), Min, Sec)
	);
}
