// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/ChatWidget/PTWChatEntry.h"
#include "Components/TextBlock.h"
#include "TimerManager.h"

void UPTWChatEntry::SetMessage(const FString& SenderName, const FString& Message)
{
	if (Text_Message)
	{
		const FString FullText = FString::Printf(TEXT("%s: %s"), *SenderName, *Message);
		Text_Message->SetText(FText::FromString(FullText));

		if (SenderName.Equals(TEXT("[System]")))
		{
			Text_Message->SetColorAndOpacity(FSlateColor(FLinearColor::Yellow));
		}
		else
		{
			Text_Message->SetColorAndOpacity(FSlateColor(FLinearColor::White));
		}
	}

	SetVisibility(ESlateVisibility::SelfHitTestInvisible);

	// 초기 생성 시 타이머 작동
	StartFadeTimer();
}

void UPTWChatEntry::SetInteractionMode(bool bIsInteracting)
{
	if (bIsInteracting)
	{
		// 채팅창이 열려 있는 동안은 무조건 보이게 함
		// 진행 중인 타이머가 있다면 취소하여 사라지는 것을 방지
		GetWorld()->GetTimerManager().ClearTimer(FadeTimerHandle);
		SetRenderOpacity(1.f);
	}
	else
	{
		// 채팅창이 닫히는 순간 모든 메세지의 타이머를 초기화 (다시 시작)
		StartFadeTimer();
	}
}

void UPTWChatEntry::StartFadeTimer()
{
	// 기존 타이머 클리어 후 재시작
	GetWorld()->GetTimerManager().ClearTimer(FadeTimerHandle);

	// 가시성 확보
	SetRenderOpacity(1.f);

	GetWorld()->GetTimerManager().SetTimer(
		FadeTimerHandle,
		this,
		&UPTWChatEntry::OnFadeOutTimeout,
		MessageDuration,
		false
	);
}

void UPTWChatEntry::OnFadeOutTimeout()
{
	SetRenderOpacity(0.f);
}
