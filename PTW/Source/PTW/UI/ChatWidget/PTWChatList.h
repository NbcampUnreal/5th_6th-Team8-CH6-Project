// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "PTWChatList.generated.h"

class UPTWChatEntry;
class UScrollBox;
class UBorder;
/**
 * 
 */
UCLASS()
class PTW_API UPTWChatList : public UUserWidget
{
	GENERATED_BODY()
	
public:
	/* 새로운 메세지를 리스트에 추가 */
	void AddChatMessage(const FString& Sender, const FString& Message);

	/* 채팅 인터랙션 모드 설정 (엔터 키 입력 시 호출) */
	void SetInteractionMode(bool bIsInteracting);

protected:
	virtual void NativeConstruct() override;
	virtual void NativeDestruct() override;

	UFUNCTION()
	void HandleChatMessage(const FString& Sender, const FString& Message);

	/* 메세지 엔트리 위젯 클래스 (에디터에서 할당) */
	UPROPERTY(EditDefaultsOnly, Category = "Chat")
	TSubclassOf<UPTWChatEntry> EntryClass;

	/* 최대 저장 가능한 메세지 개수 */
	UPROPERTY(EditDefaultsOnly, Category = "Chat")
	int32 MaxMessageCount = 50;

	/* 위젯 바인딩 */
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UScrollBox> ChatScrollBox;
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UBorder> BackgroundBorder;

private:
	bool bIsInteracting = false;
};
