// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "CoreFramework/Game/GameState/PTWGameState.h"
#include "PTWMiniGameTitle.generated.h"

class UTextBlock;
class UDataTable;

/**
 * 
 */
UCLASS()
class PTW_API UPTWMiniGameTitle : public UUserWidget
{
	GENERATED_BODY()

public:
	void InitPS();
	
protected:
	virtual void NativeConstruct() override;
	virtual void NativeDestruct() override;

	/* 미니게임 이름이 표시될 텍스트 */
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> TitleText;

	/* 맵 정보를 조회하기 위한 데이터 테이블 (에디터에서 할당) */
	UPROPERTY(EditDefaultsOnly, Category = "Data")
	TObjectPtr<UDataTable> MiniGameMapTable;

private:
	/* 델리게이트 */
	void BindGSDelegates();
	void UnBindGSDelegates();

	/* 게임 페이즈 변경 */
	UFUNCTION()
	void HandleGamePhaseChanged(EPTWGamePhase CurrentGamePhase);
	
	/* 룰렛 페이즈 변경 */
	UFUNCTION()
	void HandleRoulettePhaseChanged(FPTWRouletteData RouletteData);

	void InitializeRouletteData();

	/* RowName을 통해 실제 출력용 이름을 가져오는 헬퍼 함수 */
	FText GetMapDisplayName(FName RowName);

	/* 초기 상태 설정 */
	void InitializeTitleState();

	/* GameState가 유효할 때까지 반복 호출될 함수 */
	void TryBindGameState();

	FTimerHandle GameStateBindTimerHandle;
};
