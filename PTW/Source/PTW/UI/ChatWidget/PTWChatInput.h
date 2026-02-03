// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "PTWChatInput.generated.h"

class UEditableText;
class UPTWChatList;
/**
 * 
 */
UCLASS()
class PTW_API UPTWChatInput : public UUserWidget
{
	GENERATED_BODY()
	
public:
	/* 사용자가 엔터를 눌렀을 때 호출 (전송 로직) */
	void HandleEnterPressed();

protected:
	/* PushWidget으로 생성될 때 자동 호출 */
	virtual void NativeConstruct() override;
	/* PopWidget으로 파괴될 때 자동 호출 */
	virtual void NativeDestruct() override;

	/* OnTextCommitted에 바인딩할 함수 */
	UFUNCTION()
	void OnChatTextCommitted(const FText& Text, ETextCommit::Type CommitMethod);
private:
	/* 참조를 위해 필요한 리스트 클래스 */
	UPROPERTY(EditDefaultsOnly, Category = "Chat")
	TSubclassOf<UPTWChatList> ChatListClass;

	/* 위젯 바인딩 */
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UEditableText> Edit_ChatInput;
};
