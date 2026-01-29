// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/InGameUI/PTWKillLogEntry.h"
#include "Components/TextBlock.h"
#include "TimerManager.h"
#include "GameFramework/PlayerController.h"
#include "GameFramework/PlayerState.h"

void UPTWKillLogEntry::Init(const FString& Killer, const FString& Victim, float LifeTime)
{
	APlayerController* PC = GetOwningPlayer();

	if (PC && PC->PlayerState)
	{
		FString MyName = PC->PlayerState->GetPlayerName();

		if (KillerText)
		{
			KillerText->SetText(FText::FromString(FString::Printf(TEXT("%s "), *Killer)));
			if (Killer == MyName)
			{
				KillerText->SetColorAndOpacity(FLinearColor(0.1f, 0.1f, 0.8f, 1.0f));
			}
		}

		if (VictimText)
		{
			VictimText->SetText(FText::FromString(FString::Printf(TEXT(" %s"), *Victim)));
			if (Victim == MyName)
			{
				VictimText->SetColorAndOpacity(FLinearColor(0.8f, 0.1f, 0.1f, 1.0f));
			}
		}
		
		// KillText 는 나중에 무기종류로 바꿀예정
	}

	/* 수명 타이머 시작 */
	GetWorld()->GetTimerManager().SetTimer(
		LifeTimerHandle,
		this,
		&UPTWKillLogEntry::HandleExpired,
		LifeTime,
		false
	);
}

void UPTWKillLogEntry::HandleExpired()
{
	/* 제거는 PTWKillLogUI가 담당 */
	OnExpired.Broadcast(this);
}
