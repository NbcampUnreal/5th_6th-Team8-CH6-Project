// Fill out your copyright notice in the Description page of Project Settings.


#include "MiniGame/Manager/PTWRoundEventManager.h"

#include "CoreFramework/Game/GameMode/PTWLobbyGameMode.h"
#include "CoreFramework/Game/GameState/PTWGameState.h"
#include "MiniGame/PTWMiniGameMapRow.h"
#include "MiniGame/Data/PTWRoundEvent.h"
#include "System/Shop/PTWShopSubsystem.h"


void UPTWRoundEventManager::StartRouletteSequence()
{
	SelectedRandomMap();
	SelectedRandomEvent();

	StartMapRoulette();
}

void UPTWRoundEventManager::BeginPlay()
{
	Super::BeginPlay();

	if (UWorld* World = GetWorld())
	{
		PTWGameState = World->GetGameState<APTWGameState>();
	}
}

void UPTWRoundEventManager::SelectedRandomMap()
{
	if (!PTWGameState) return;
	if (!MiniGameMapTable) return;
	
	TArray<FName> SelectableRowNames = GetSelectableMapRowNames();

	if (SelectableRowNames.Num() == 0) return;
	
	const int32 RandomIndex = FMath::RandRange(0, SelectableRowNames.Num()-1);
	const FName SelectedRowName = SelectableRowNames[RandomIndex];

	const FPTWMiniGameMapRow* Row = MiniGameMapTable->FindRow<FPTWMiniGameMapRow>(SelectedRowName, TEXT("Map"));

	if (Row)
	{
		TravelLevelName = Row->Map.ToSoftObjectPath().GetLongPackageName();
		MapTag = Row->MiniGameTag;
		GEngine->AddOnScreenDebugMessage(0, 3.f, FColor::Black, Row->DisplayName.ToString(), false);
	}

	FPTWRouletteData RouletteData = PTWGameState->GetRouletteData();
	RouletteData.MapRowName = SelectedRowName;
	PTWGameState->SetRouletteData(RouletteData); // GameState에 전달
}

void UPTWRoundEventManager::SelectedRandomEvent()
{
	if (!PTWGameState) return;
	if (!LobbyRoundEventTable) return;

	TArray<FName> RowNames = LobbyRoundEventTable->GetRowNames();

	if (RowNames.Num() ==0) return;

	const int32 RandomIndex = FMath::RandRange(0, RowNames.Num()-1);
	const FName SelectedRowName = RowNames[RandomIndex];

	const FPTWRoundEventRow* Row = LobbyRoundEventTable->FindRow<FPTWRoundEventRow>(SelectedRowName, TEXT("Round Event"));
	
	if (Row)
	{
		EventTag = Row->EventTag;
		GEngine->AddOnScreenDebugMessage(0, 3.f, FColor::Black, Row->EventName.ToString());
	}
	
	FPTWRouletteData RouletteData = PTWGameState->GetRouletteData();
	RouletteData.EventRowName = SelectedRowName;
	PTWGameState->SetRouletteData(RouletteData);
}

TArray<FName> UPTWRoundEventManager::GetSelectableMapRowNames()
{
	TArray<FName> SelectableRowNames;
	
	if (!PTWGameState)
	{
		return SelectableRowNames;
	}
	if (!MiniGameMapTable)
	{
		return SelectableRowNames;
	}

	int32 PlayerCount = PTWGameState->PlayerArray.Num();
	TArray<FName> RowNames = MiniGameMapTable->GetRowNames();

	SelectableRowNames.Reserve(RowNames.Num());

	// 모든 Row를 순회하면서 선택 가능 여부를 검사
	for (const FName& RowName : RowNames)
	{
		const FPTWMiniGameMapRow* Row = MiniGameMapTable->FindRow<FPTWMiniGameMapRow>(RowName, TEXT("Map"));

		if (!Row) continue;
		
		if (Row->MinPlayers <= PlayerCount && Row->MaxPlayers >= PlayerCount)
		{
			// 조건을 만족하면 선택 가능한 후보로 추가
			SelectableRowNames.Add(RowName);
		}
	}
	// 조건을 만족하는 모든 맵 RowName 반환
	return SelectableRowNames;
}

void UPTWRoundEventManager::StartMapRoulette()
{
	if (!PTWGameState) return;

	FPTWRouletteData RouletteData = PTWGameState->GetRouletteData();
	RouletteData.CurrentPhase = EPTWRoulettePhase::MapRoulette;
	RouletteData.RouletteDuration = 5.f;
	PTWGameState->SetRouletteData(RouletteData);

	UWorld* World = GetWorld();
	if (!World) return;
		
	World->GetTimerManager().SetTimer(RouletteTimer,this ,&UPTWRoundEventManager::StartRoundEventRoulette, 5.f);
}

void UPTWRoundEventManager::StartRoundEventRoulette()
{
	if (!PTWGameState) return;
	
	FPTWRouletteData RouletteData = PTWGameState->GetRouletteData();
	RouletteData.CurrentPhase = EPTWRoulettePhase::RoundEventRoulette;
	RouletteData.RouletteDuration = 5.f;
	PTWGameState->SetRouletteData(RouletteData);

	UWorld* World = GetWorld();
	if (!World) return;
	
	World->GetTimerManager().SetTimer(RouletteTimer, this, &UPTWRoundEventManager::EndRoulette, 5.f);
	
}

void UPTWRoundEventManager::EndRoulette()
{
	if (!PTWGameState) return;
	
	FPTWRouletteData RouletteData = PTWGameState->GetRouletteData();
	RouletteData.CurrentPhase = EPTWRoulettePhase::Finished;
	PTWGameState->SetRouletteData(RouletteData);
	
	if (UPTWShopSubsystem* ShopSubsystem = GetWorld()->GetSubsystem<UPTWShopSubsystem>())
	{
		ShopSubsystem->InitializeShopsForRound(MapTag, EventTag);
	}

	FName SelectedMapRowName = PTWGameState->GetRouletteData().MapRowName;
	
	UWorld* World = GetWorld();
	if (!World) return;
	
	OnRouletteFinished.Broadcast(SelectedMapRowName);
}
