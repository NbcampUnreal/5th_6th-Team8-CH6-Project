// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/InGameUI/PTWTimer.h"
#include "CoreFramework/Game/GameState/PTWGameState.h"
#include "Components/TextBlock.h"
#include "Kismet/GameplayStatics.h"

void UPTWTimer::InitTimer()
{
	PTWGameState = GetWorld() ? GetWorld()->GetGameState<APTWGameState>() : nullptr;

	if (IsValid(PTWGameState))
	{
		// 중복 바인딩 방지 
		PTWGameState->OnRemainTimeChanged.RemoveDynamic(this, &UPTWTimer::HandleRemainTimeChanged);
		PTWGameState->OnMiniGameCountdownChanged.RemoveDynamic(this, &UPTWTimer::MiniGameCountdownChanged);

		GetWorld()->GetTimerManager().ClearTimer(TimerHandle_InitGameState);

		UE_LOG(LogTemp, Warning, TEXT("PTWTimer : InitTimer"));

		// 델리게이트 바인딩
		PTWGameState->OnRemainTimeChanged.AddDynamic(this, &UPTWTimer::HandleRemainTimeChanged);
		PTWGameState->OnMiniGameCountdownChanged.AddDynamic(this, &UPTWTimer::MiniGameCountdownChanged);

		// 초기 값 반영
		HandleRemainTimeChanged(PTWGameState->GetRemainTime());
		MiniGameCountdownChanged(PTWGameState->IsMiniGameCountdown());
	}
	else
	{
		if (!TimerHandle_InitGameState.IsValid())
		{
			GetWorld()->GetTimerManager().SetTimer(
				TimerHandle_InitGameState,
				this,
				&UPTWTimer::InitTimer,
				0.1f,
				true
			);
		}
	}
}

void UPTWTimer::NativeConstruct()
{
	Super::NativeConstruct();
}

void UPTWTimer::NativeDestruct()
{
	if (PTWGameState)
	{
		PTWGameState->OnRemainTimeChanged.RemoveDynamic(
			this, &UPTWTimer::HandleRemainTimeChanged
		);

		PTWGameState->OnMiniGameCountdownChanged.RemoveDynamic(
			this, &UPTWTimer::MiniGameCountdownChanged
		);
	}

	Super::NativeDestruct();
}

void UPTWTimer::HandleRemainTimeChanged(int32 NewRemainTime)
{
	if (!RemainTimeText) return;

	RemainTimeText->SetText(FormatTime(NewRemainTime));
}

void UPTWTimer::MiniGameCountdownChanged(bool iscountdown)
{
	// false 면 visible
	if (!RemainTimeText) return;

	if (iscountdown)
	{
		RemainTimeText->SetVisibility(ESlateVisibility::Hidden);
	}
	else
	{
		RemainTimeText->SetVisibility(ESlateVisibility::Visible);
	}
}

FText UPTWTimer::FormatTime(int32 Seconds) const
{
	const int32 Min = Seconds / 60;
	const int32 Sec = Seconds % 60;

	return FText::FromString(
		FString::Printf(TEXT("%02d:%02d"), Min, Sec)
	);
}
