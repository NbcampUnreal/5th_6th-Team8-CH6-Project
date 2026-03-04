// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/ChatWidget/PTWChatList.h"
#include "UI/ChatWidget/PTWChatEntry.h"
#include "Components/ScrollBox.h"
#include "Components/Image.h"
#include "Components/Border.h"
#include "CoreFramework/Game/GameState/PTWGameState.h"

void UPTWChatList::AddChatMessage(const FString& Sender, const FString& Message)
{
	if (!EntryClass || !ChatScrollBox) return;

	if (GetVisibility() == ESlateVisibility::Hidden)
	{
		SetVisibility(ESlateVisibility::SelfHitTestInvisible);
	}

	 /* 50개 제한 체크: 초과 시 가장 오래된 메세지(인덱스 0) 제거 */
	if (ChatScrollBox->GetChildrenCount() >= MaxMessageCount)
	{
		ChatScrollBox->RemoveChildAt(0);
	}

	/* 새로운 엔트리 생성 및 추가 */
	if (APlayerController* PC = GetOwningPlayer())
	{
		if (UPTWChatEntry* NewEntry = CreateWidget<UPTWChatEntry>(PC, EntryClass))
		{
			if (IsValid(ChatScrollBox))
			{
				ChatScrollBox->AddChild(NewEntry);
				NewEntry->SetMessage(Sender, Message);

				NewEntry->SetInteractionMode(bIsInteracting);

				/* 항상 최신 메세지가 보이도록 스크롤 하단 이동 */
				ChatScrollBox->ScrollToEnd();
			}
		}
	}
}

void UPTWChatList::SetInteractionMode(bool bInInteracting)
{
	bIsInteracting = bInInteracting;

	/* 배경 이미지 가시성 조절 (평소엔 투명하다가 열리면 보임) */
	if (BackgroundBorder)
	{
		BackgroundBorder->SetVisibility(bIsInteracting ? ESlateVisibility::Visible : ESlateVisibility::Hidden);
	}

	/* 스크롤바 제어(열려있을 때만 스크롤 가능) */
	if (ChatScrollBox)
	{
		ChatScrollBox->SetScrollBarVisibility(bIsInteracting ? ESlateVisibility::Visible : ESlateVisibility::Collapsed);
		ChatScrollBox->SetConsumeMouseWheel(bIsInteracting ? EConsumeMouseWheel::Always : EConsumeMouseWheel::Never);
	}

	/* 모든 자식 엔트리들에게 상태 전파(타이머 초기화 로직 등 실행) */
	for (UWidget* Child : ChatScrollBox->GetAllChildren())
	{
		if (UPTWChatEntry* Entry = Cast<UPTWChatEntry>(Child))
		{
			Entry->SetInteractionMode(bIsInteracting);
		}
	}
}

void UPTWChatList::NativeConstruct()
{
	Super::NativeConstruct();

	if (BackgroundBorder)
	{
		BackgroundBorder->SetVisibility(ESlateVisibility::Hidden);
	}

	if (ChatScrollBox)
	{
		ChatScrollBox->SetScrollBarVisibility(ESlateVisibility::Collapsed);
		ChatScrollBox->SetConsumeMouseWheel(EConsumeMouseWheel::Never);
		ChatScrollBox->ScrollToEnd();
	}

	if (APTWGameState* GS = GetWorld()->GetGameState<APTWGameState>())
	{
		GS->OnChatMessageBroadcast.AddDynamic(this, &UPTWChatList::HandleChatMessage);
	}
}

void UPTWChatList::NativeDestruct()
{
	if (UWorld* World = GetWorld())
	{
		if (APTWGameState* GS = World->GetGameState<APTWGameState>())
		{
			GS->OnChatMessageBroadcast.RemoveDynamic(this, &UPTWChatList::HandleChatMessage);
		}
	}

	Super::NativeDestruct();
}

void UPTWChatList::HandleChatMessage(const FString& Sender, const FString& Message)
{
	UWorld* World = GetWorld();
	if (!IsValid(this) || !World || World->bIsTearingDown)
	{
		return;
	}

	AddChatMessage(Sender, Message);
}
