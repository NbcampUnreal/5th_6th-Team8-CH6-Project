// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "PTWPlayerController.generated.h"

DECLARE_MULTICAST_DELEGATE_TwoParams(FOnKillLog, const FString&, const FString&);

class APTWHUD;
class APTWPlayerState;
class UAbilitySystemComponent;
class UInputMappingContext;
class UInputAction;
class UPTWRankingBoard;
/**
 * 
 */
UCLASS()
class PTW_API APTWPlayerController : public APlayerController
{
	GENERATED_BODY()
	
public:
	virtual void BeginPlay() override;

	virtual void OnRep_PlayerState() override;

	virtual void BeginSpectatingState() override;
	
	virtual void OnRep_Pawn() override;
	
	void TryInitializeHUD();
	
	void StartSpectating();
	UFUNCTION()
	void SpectateNextPlayer();
	UFUNCTION()
	void OnInputSpectateNext();
	
	// KillLog 델리게이트
	FOnKillLog OnKillLog;

protected:
	virtual void SetupInputComponent() override;

	/* 랭킹보드 펼치기 */
	void OnRankingPressed();
	void OnRankingReleased();

	virtual void PostSeamlessTravel() override;

	void CreateRankingBoard();

	/* ---------- Input ---------- */

	UPROPERTY(EditDefaultsOnly, Category = "Input")
	UInputMappingContext* DefaultMappingContext;

	UPROPERTY(EditDefaultsOnly, Category = "Input")
	UInputAction* ShowRankingAction;

	UPROPERTY(EditDefaultsOnly, Category = "Input")
	UInputAction* SpectateNextAction;
	
	/* ---------- UI ---------- */

	UPROPERTY(EditDefaultsOnly, Category = "UI")
	TSubclassOf<UPTWRankingBoard> RankingBoardClass;

	UPROPERTY()
	TObjectPtr<UPTWRankingBoard> RankingBoard;
};
