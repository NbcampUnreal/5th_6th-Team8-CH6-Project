// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "PTWChatEntry.generated.h"

class UTextBlock;
/**
 * 
 */
UCLASS()
class PTW_API UPTWChatEntry : public UUserWidget
{
	GENERATED_BODY()

public:
	/* 메세지 데이터 설정 */
	void SetMessage(const FString& SenderName, const FString& Message);

	/* 채팅창 상태 변화 알림 (열림/닫힘) */
	void SetInteractionMode(bool bIsInteracting);

protected:
	/* 5초 타이머 시작 (기존 타이머가 있으면 초기화 후 재시작) */
	void StartFadeTimer();

	/* 타이머 종료 시 호출 */
	void OnFadeOutTimeout();

private:
	/* 타이머 핸들 */
	FTimerHandle FadeTimerHandle;

	/* 메세지 유지 시간 (에디터에서 수정 가능) */
	UPROPERTY(EditDefaultsOnly, Category = "Chat")
	float MessageDuration = 5.0f;

	/* 위젯 바인딩 */
	/*UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> Text_Sender;*/
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> Text_Message;
};
