// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/InGameUI/PTWKillLogEntry.h"
#include "Components/TextBlock.h"
#include "TimerManager.h"

void UPTWKillLogEntry::Init(const FString& Killer, const FString& Victim, float LifeTime)
{
	if (KillText)
	{
		KillText->SetText(
			FText::FromString(
				FString::Printf(TEXT("%s killed %s"), *Killer, *Victim)));
	} // 킬로그의 killed 는 나중에 무기종류로 바꿀예정

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
