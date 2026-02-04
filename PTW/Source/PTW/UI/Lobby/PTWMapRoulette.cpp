// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/Lobby/PTWMapRoulette.h"
#include "Components/TextBlock.h"
#include "Engine/DataTable.h"

#include "MiniGame/PTWMiniGameMapRow.h"
#include "CoreFramework/Game/GameState/PTWGameState.h"

void UPTWMapRoulette::NativeConstruct()
{
	Super::NativeConstruct();

	// 테이블에서 맵 이름들 캐싱
	CacheMapNames();

	if (APTWGameState* GS = GetWorld()->GetGameState<APTWGameState>())
	{
		// 셔플 애니메이션 시작 (ShuffleInterval 간격으로 텍스트 변경)
		GetWorld()->GetTimerManager().SetTimer(ShuffleTimerHandle, this, &ThisClass::PlayShuffleAnim, ShuffleInterval, true);

		// GameMode에서 설정한 RouletteDuration만큼 대기 후 최종 결과 표시
		float TotalTime = FMath::Max(0.5f, GS->GetRouletteData().RouletteDuration - 1.f);

		FTimerHandle ResultTimer;
		GetWorld()->GetTimerManager().SetTimer(ResultTimer, this, &ThisClass::ShowFinalResult, TotalTime, false);
	}
}

void UPTWMapRoulette::NativeDestruct()
{
	GetWorld()->GetTimerManager().ClearTimer(ShuffleTimerHandle);
	Super::NativeDestruct();
}

void UPTWMapRoulette::CacheMapNames()
{
	if (!MiniGameMapTable) return;

	AllMapNames.Empty();
	static const FString ContextString(TEXT("MapRouletteContext"));

	TArray<FPTWMiniGameMapRow*> Rows;
	MiniGameMapTable->GetAllRows<FPTWMiniGameMapRow>(ContextString, Rows);

	for (const auto* Row : Rows)
	{
		if (Row)
		{
			AllMapNames.Add(Row->DisplayName);
		}
	}
}

void UPTWMapRoulette::PlayShuffleAnim()
{
	if (AllMapNames.Num() == 0 || !MapNameText) return;

	// 랜덤하게 맵 이름 선택
	int32 RandomIndex = FMath::RandRange(0, AllMapNames.Num() - 1);
	MapNameText->SetText(AllMapNames[RandomIndex]);

	// 디자인 요소가 없으므로 텍스트 색상을 랜덤하게 바꿔서 "돌아가는 느낌" 부여
	MapNameText->SetColorAndOpacity(FSlateColor(FLinearColor::MakeRandomColor()));
}

void UPTWMapRoulette::ShowFinalResult()
{
	// 셔플 타이머 중지
	GetWorld()->GetTimerManager().ClearTimer(ShuffleTimerHandle);

	APTWGameState* GS = GetWorld()->GetGameState<APTWGameState>();
	if (!GS || !MiniGameMapTable || !MapNameText) return;

	// GameState에 이미 저장되어 있는 서버 확정 결과(MapRowName)를 가져옴
	FName TargetRowName = GS->GetRouletteData().MapRowName;
	static const FString ContextString(TEXT("FinalResultContext"));

	FPTWMiniGameMapRow* ResultRow = MiniGameMapTable->FindRow<FPTWMiniGameMapRow>(TargetRowName, ContextString);
	if (ResultRow)
	{
		// 최종 결과 이름 설정
		MapNameText->SetText(ResultRow->DisplayName);

		// 최종 결과는 눈에 잘 띄는 색상(예: 노란색)으로 고정
		MapNameText->SetColorAndOpacity(FSlateColor(FLinearColor::Yellow));

		// 여기서 결과 당첨 사운드 등을 플레이
	}
}
