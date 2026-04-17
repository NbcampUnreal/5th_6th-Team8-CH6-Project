// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/InGameUI/PTWPortalCount.h"
#include "Components/TextBlock.h"
#include "CoreFramework/Game/GameState/PTWGameState.h"
#include "AbilitySystemComponent.h"
#include "AbilitySystemInterface.h"
#include "PTWGameplayTag/GameplayTags.h"
#include "GameFramework/PlayerState.h"

#define LOCTEXT_NAMESPACE "PTWPortalUI"

void UPTWPortalCount::NativeDestruct()
{
	if (PTWGameState)
	{
		PTWGameState->OnPortalCountChanged.RemoveDynamic(this, &UPTWPortalCount::UpdatePortalText);
	}

	Super::NativeDestruct();
}

void UPTWPortalCount::InitWithASC(UAbilitySystemComponent* AbilitySystemComponent)
{
	if (AbilitySystemComponent)
	{
		ASC = AbilitySystemComponent;

		ASC->RegisterGameplayTagEvent(GameplayTags::State::InPortal, EGameplayTagEventType::NewOrRemoved)
			.AddUObject(this, &UPTWPortalCount::OnPortalTagChanged);
	}

	InitializeGameState();
}

void UPTWPortalCount::InitializeGameState()
{
	PTWGameState = GetWorld() ? GetWorld()->GetGameState<APTWGameState>() : nullptr;

	if (IsValid(PTWGameState))
	{
		// 중복 바인딩 방지 
		PTWGameState->OnPortalCountChanged.RemoveDynamic(this, &UPTWPortalCount::UpdatePortalText);

		GetWorld()->GetTimerManager().ClearTimer(PortalCount_InitGameState);

		UE_LOG(LogTemp, Warning, TEXT("PTWTimer : InitTimer"));

		// 델리게이트 바인딩
		PTWGameState->OnPortalCountChanged.AddDynamic(this, &UPTWPortalCount::UpdatePortalText);

		// 초기 값 반영
		UpdatePortalText(PTWGameState->GetPortalCurrent(),PTWGameState->GetPortalRequired());
	}
	else
	{
		if (!PortalCount_InitGameState.IsValid())
		{
			GetWorld()->GetTimerManager().SetTimer(
				PortalCount_InitGameState,
				this,
				&UPTWPortalCount::InitializeGameState,
				0.1f,
				true
			);
		}
	}
}

void UPTWPortalCount::UpdatePortalText(int32 Current, int32 Required)
{
	if (PortalCountText)
	{
		APlayerController* PC = GetOwningPlayer();
		if (!PC) return;

		APlayerState* PS = PC->PlayerState;
		if (!PS) return;

		if (!ASC) return;

		bool bIsInPortal = ASC && ASC->HasMatchingGameplayTag(GameplayTags::State::InPortal);

		if (bIsInPortal)
		{
			FText FormattedText = FText::Format(
				LOCTEXT("PortalCountFormat", "{0} / {1}"),
				FText::AsNumber(Current),
				FText::AsNumber(Required)
			);

			PortalCountText->SetText(FormattedText);
		}
		else
		{
			PortalCountText->SetText(
				LOCTEXT("PortalMoveMessage", "포탈로 이동하여 준비완료를 하세요")
			);
		}
	}
}

void UPTWPortalCount::OnPortalTagChanged(const FGameplayTag Tag, int32 NewCount)
{
	if (PTWGameState)
	{
		UpdatePortalText(PTWGameState->GetPortalCurrent(), PTWGameState->GetPortalRequired());
	}
}

#undef LOCTEXT_NAMESPACE
