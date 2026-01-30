// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "PTWRankingBoard.generated.h"

class UVerticalBox;
class UPTWRankingEntry;
class APTWPlayerState;
class UTextBlock;

/**
 * 
 */
UCLASS()
class PTW_API UPTWRankingBoard : public UUserWidget
{
	GENERATED_BODY()
	
public:
	/* 랭킹 갱신 */
	UFUNCTION()
	void UpdateRanking();

protected:
	virtual void NativeConstruct() override;
	virtual void NativeDestruct() override;

	/* PlayerState 델리게이트 바인딩 */
	void BindPlayerStates();
	void UnbindPlayerStates();

	UFUNCTION()
	void OnPlayerDataChanged(const FPTWPlayerData& NewData);

	// 상단 미니게임 제목 표시용
	UPROPERTY(meta = (BindWidget))
	UTextBlock* Text_GameTitle;
	// 하단 랭킹 엔트리 출력용
	UPROPERTY(meta = (BindWidget))
	UVerticalBox* RankingList;

	/* 페이즈별 위젯 클래스 설정 (에디터에서 할당) */
	UPROPERTY(EditDefaultsOnly, Category = "Ranking|Classes")
	TSubclassOf<UPTWRankingEntry> PreGameEntryClass;
	UPROPERTY(EditDefaultsOnly, Category = "Ranking|Classes")
	TSubclassOf<UPTWRankingEntry> MiniGameEntryClass;
	UPROPERTY(EditDefaultsOnly, Category = "Ranking|Classes")
	TSubclassOf<UPTWRankingEntry> PostGameEntryClass;
	/* 페이즈별 상단 헤더 위젯 클래스 (에디터에서 할당) */
	UPROPERTY(EditDefaultsOnly, Category = "Ranking|Header")
	TSubclassOf<UUserWidget> PreGameHeaderClass;
	UPROPERTY(EditDefaultsOnly, Category = "Ranking|Header")
	TSubclassOf<UUserWidget> MiniGameHeaderClass;
	UPROPERTY(EditDefaultsOnly, Category = "Ranking|Header")
	TSubclassOf<UUserWidget> PostGameHeaderClass;

	/* 캐싱 */
	UPROPERTY()
	TArray<TObjectPtr<APTWPlayerState>> CachedPlayerStates;
};
