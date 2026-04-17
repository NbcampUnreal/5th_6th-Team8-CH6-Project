// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/Predict/PTWPredictWinVote.h"
#include "PTWPredictPlayerEntry.h"

#include "Components/VerticalBox.h"
#include "Components/TextBlock.h"
#include "Components/Button.h"
#include "GameFramework/GameStateBase.h"

#include "CoreFramework/PTWPlayerState.h"
#include "UI/PTWUISubsystem.h"

void UPTWPredictWinVote::NativeConstruct()
{
	Super::NativeConstruct();

	if (Btn_OK)
	{
		Btn_OK->OnClicked.AddDynamic(this, &UPTWPredictWinVote::OnOKClicked);
	}
	if (Btn_Cancel)
	{
		Btn_Cancel->OnClicked.AddDynamic(this, &UPTWPredictWinVote::OnCancelClicked);
	}

	// 플레이어 목록 가져오기 
	AGameStateBase* GS = GetWorld()->GetGameState();
	if (!GS || !PlayerButtonList || !PlayerEntryClass) return;

	PlayerButtonList->ClearChildren();

	for (APlayerState* PS : GS->PlayerArray)
	{
		if (APTWPlayerState* PTWPS = Cast<APTWPlayerState>(PS))
		{
			// 엔트리 위젯 생성
			UPTWPredictPlayerEntry* Entry = CreateWidget<UPTWPredictPlayerEntry>(this, PlayerEntryClass);
			if (Entry)
			{
				PlayerButtonList->AddChild(Entry);

				Entry->SetEntryData(PTWPS->GetPlayerName());
				Entry->OnClickedDelegate.BindUObject(this, &UPTWPredictWinVote::SetSelectedPlayer);
			}
		}
	}
}

void UPTWPredictWinVote::SetSelectedPlayer(const FString& PlayerName)
{
	CurrentSelectedName = PlayerName;
	if (Text_SelectedPlayer)
	{
		Text_SelectedPlayer->SetText(FText::FromString(PlayerName));
	}
}

void UPTWPredictWinVote::OnOKClicked()
{
	if (CurrentSelectedName.IsEmpty()) return;

	// PlayerState에 저장
	APTWPlayerState* MyPS = GetOwningPlayerState<APTWPlayerState>();
	if (MyPS)
	{
		MyPS->VotePredictedPlayer(MyPS->GetUniqueId());
	}

	// UI 닫기
	if (ULocalPlayer* LP = GetOwningLocalPlayer())
	{
		if (UPTWUISubsystem* UISubsystem = LP->GetSubsystem<UPTWUISubsystem>())
		{
			UISubsystem->PopWidget();
		}
	}
}

void UPTWPredictWinVote::OnCancelClicked()
{
	if (ULocalPlayer* LP = GetOwningLocalPlayer())
	{
		if (UPTWUISubsystem* UISubsystem = LP->GetSubsystem<UPTWUISubsystem>())
		{
			UISubsystem->PopWidget();
		}
	}
}
