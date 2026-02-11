// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/InGameUI/PTWKillLogUI.h"
#include "Components/VerticalBox.h"
#include "PTWKillLogEntry.h"
#include "CoreFramework/Game/GameState/PTWGameState.h"
#include "CoreFramework/PTWPlayerState.h"
#include "MiniGame/Item/BombItem/PTWBombActor.h"
#include "MiniGame/GameMode/BombMiniGameMode/PTWBombMiniGameMode.h"

void UPTWKillLogUI::NativeConstruct()
{
	Super::NativeConstruct();

	BindGameStates();
}

void UPTWKillLogUI::NativeDestruct()
{
	UnbindGameStates();

	Super::NativeDestruct();
}

void UPTWKillLogUI::BindGameStates()
{
	if (APTWGameState* GS = GetWorld()->GetGameState<APTWGameState>())
	{
		GS->OnKilllogBroadcast.AddDynamic(this, &UPTWKillLogUI::OnKilllogReceived);
		
		GS->OnKilllogBroadcastEx.AddDynamic(this, &UPTWKillLogUI::OnKilllogReceivedEx);
	}
}

void UPTWKillLogUI::UnbindGameStates()
{
	if (APTWGameState* GS = GetWorld()->GetGameState<APTWGameState>())
	{
		GS->OnKilllogBroadcast.RemoveAll(this);
		
		GS->OnKilllogBroadcastEx.RemoveAll(this);
	}
}

void UPTWKillLogUI::OnKilllogReceived(AActor* DeadActor, AActor* KillerActor)
{
	//폭탄 게임 모드에서는 원인 포함하는 킬로그만
	if (GetWorld() && GetWorld()->GetAuthGameMode<APTWBombMiniGameMode>())
	{
		return;
	}
	// Actor가 유효한지 확인 후 이름 추출
	FString KillerName = TEXT("Unknown");
	if (APTWPlayerState* KPS = Cast<APTWPlayerState>(KillerActor))
	{
		FPTWPlayerData KillerData = KPS->GetPlayerData();
		if (!KillerData.PlayerName.IsEmpty()) KillerName = KillerData.PlayerName;
		else KillerName = KPS->GetPlayerName();
	}

	FString VictimName = TEXT("Unknown");
	if (APTWPlayerState* VPS = Cast<APTWPlayerState>(DeadActor))
	{
		FPTWPlayerData VictimData = VPS->GetPlayerData();
		if (!VictimData.PlayerName.IsEmpty()) VictimName = VictimData.PlayerName;
		else VictimName = VPS->GetPlayerName();
	}

	AddKillLog(KillerName, VictimName);
}

void UPTWKillLogUI::OnKilllogReceivedEx(AActor* DeadActor, AActor* KillerActor, FName CauseId)
{
	FString KillerName = TEXT("Unknown");
	if (APTWPlayerState* KPS = Cast<APTWPlayerState>(KillerActor))
	{
		const FPTWPlayerData KillerData = KPS->GetPlayerData();
		KillerName = !KillerData.PlayerName.IsEmpty() ? KillerData.PlayerName : KPS->GetPlayerName();
	}

	FString VictimName = TEXT("Unknown");
	if (APTWPlayerState* VPS = Cast<APTWPlayerState>(DeadActor))
	{
		const FPTWPlayerData VictimData = VPS->GetPlayerData();
		VictimName = !VictimData.PlayerName.IsEmpty() ? VictimData.PlayerName : VPS->GetPlayerName();
	}
	
	AddKillLogWithCause(KillerName, VictimName, CauseId.ToString());
}

void UPTWKillLogUI::AddKillLog(const FString& Killer, const FString& Victim)
{
	if (!KillLogEntryClass || !LogList)
		return;

	UPTWKillLogEntry* NewEntry = CreateWidget<UPTWKillLogEntry>(GetWorld(), KillLogEntryClass);

	if (!NewEntry)
		return;

	/* 만료 델리게이트 바인딩 */
	NewEntry->OnExpired.AddUObject(this, &UPTWKillLogUI::HandleEntryExpired);

	NewEntry->Init(Killer, Victim, LogLifeTime);

	/* 최신 로그를 위로*/
	LogList->InsertChildAt(0, NewEntry);
	ActiveEntries.Insert(NewEntry, 0);

	/* 개수 초과 시 가장 오래된 로그 제거 */
	if (ActiveEntries.Num() > MaxLogCount)
	{
		RemoveOldest();
	}
}

void UPTWKillLogUI::AddKillLogWithCause(const FString& Killer, const FString& Victim, const FString& CauseText)
{
	if (!KillLogEntryClass || !LogList)
		return;

	UPTWKillLogEntry* NewEntry = CreateWidget<UPTWKillLogEntry>(GetWorld(), KillLogEntryClass);
	if (!NewEntry)
		return;

	/* 만료 델리게이트 바인딩 */
	NewEntry->OnExpired.AddUObject(this, &UPTWKillLogUI::HandleEntryExpired);

	// 무기 표시 버전
	NewEntry->InitWithCause(Killer, Victim, CauseText, LogLifeTime);
	
	LogList->InsertChildAt(0, NewEntry);
	ActiveEntries.Insert(NewEntry, 0);
	
	if (ActiveEntries.Num() > MaxLogCount)
	{
		RemoveOldest();
	}
}

void UPTWKillLogUI::RemoveOldest()
{
	if (ActiveEntries.Num() == 0) return;

	TObjectPtr<UPTWKillLogEntry> Oldest = ActiveEntries.Last();

	ActiveEntries.RemoveAt(ActiveEntries.Num() - 1);

	if (Oldest)
	{
		Oldest->RemoveFromParent();
	}
}

void UPTWKillLogUI::HandleEntryExpired(UPTWKillLogEntry* Entry)
{
	if (!Entry)	return;

	ActiveEntries.Remove(Entry);
	Entry->RemoveFromParent();
}
