// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/MiniGame/PTWGameStartTimer.h"
#include "CoreFramework/Game/GameState/PTWGameState.h"
#include "Components/TextBlock.h"
#include "Kismet/GameplayStatics.h"

void UPTWGameStartTimer::NativeConstruct()
{
	Super::NativeConstruct();

	PTWGameState = GetWorld() ? GetWorld()->GetGameState<APTWGameState>() : nullptr;

	if (PTWGameState)
	{
		// 델리게이트 바인딩
		PTWGameState->OnMiniGameCountdownValueChanged.AddDynamic(this, &UPTWGameStartTimer::HandleCountDownChanged);

		// 초기 값 반영
		HandleCountDownChanged(PTWGameState->GetMiniGameCountDown());
	}
}

void UPTWGameStartTimer::NativeDestruct()
{
	if (PTWGameState)
	{
		PTWGameState->OnMiniGameCountdownValueChanged.RemoveDynamic(this, &UPTWGameStartTimer::HandleCountDownChanged);
	}

	Super::NativeDestruct();
}

void UPTWGameStartTimer::HandleCountDownChanged(int32 CurrentCountdown)
{
	if (!CountdownText) return;

	CountdownText->SetText(FText::AsNumber(CurrentCountdown));
}
