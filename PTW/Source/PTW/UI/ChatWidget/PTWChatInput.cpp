// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/ChatWidget/PTWChatInput.h"
#include "UI/ChatWidget/PTWChatList.h"
#include "UI/PTWUISubsystem.h"
#include "Components/EditableText.h"
#include "CoreFramework/PTWPlayerController.h"

void UPTWChatInput::HandleEnterPressed()
{
	if (!Edit_ChatInput) return;

	FString Message = Edit_ChatInput->GetText().ToString();
	Message.TrimStartAndEndInline();

	Edit_ChatInput->SetText(FText::GetEmpty());

	// 메세지가 있으면 서버로 전송
	if (!Message.IsEmpty())
	{
		if (APTWPlayerController* PC = Cast<APTWPlayerController>(GetOwningPlayer()))
		{
			PC->Server_SendChatMessage(Message);
		}
	}

	// 전송 후 입력창 닫기
	if (APTWPlayerController* PC = Cast<APTWPlayerController>(GetOwningPlayer()))
	{
		if (UPTWUISubsystem* UISub = PC->GetLocalPlayer()->GetSubsystem<UPTWUISubsystem>())
		{
			UISub->PopWidget();
		}
	}
}

void UPTWChatInput::NativeConstruct()
{
	Super::NativeConstruct();

	if (Edit_ChatInput)
	{
		/* 나타나자마자 입력창에 포커스(즉시 타이핑 가능) */
		Edit_ChatInput->SetKeyboardFocus();
		Edit_ChatInput->SetText(FText::GetEmpty());

		Edit_ChatInput->OnTextCommitted.AddDynamic(this, &UPTWChatInput::OnChatTextCommitted);
	}
}

void UPTWChatInput::NativeDestruct()
{
	Super::NativeDestruct();
}

void UPTWChatInput::OnChatTextCommitted(const FText& Text, ETextCommit::Type CommitMethod)
{
	if (CommitMethod == ETextCommit::OnEnter)
	{
		HandleEnterPressed();
	}
}
