// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "GameplayTagContainer.h"
#include "PTWPlayerController.generated.h"

/* KillLog 델리게이트 */
DECLARE_MULTICAST_DELEGATE_TwoParams(FOnKillLog, const FString&, const FString&);

class APTWHUD;
class APTWPlayerState;
class UAbilitySystemComponent;
class UInputMappingContext;
class UInputAction;
class UPTWRankingBoard;
class UPTWItemInstance;
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

	virtual void OnPossess(APawn* InPawn) override;
	virtual void OnUnPossess() override;

	/* HUD 초기화 */
	void TryInitializeHUD();

	/* KillLog 델리게이트 */
	FOnKillLog OnKillLog;

protected:
	virtual void SetupInputComponent() override;

	/* 랭킹보드 */
	void OnRankingPressed();
	void OnRankingReleased();
	void CreateRankingBoard();

	virtual void PostSeamlessTravel() override;

	/* ---------- Input ---------- */
	// RankBoard
	UPROPERTY(EditDefaultsOnly, Category = "Input")
	UInputMappingContext* DefaultMappingContext;
	UPROPERTY(EditDefaultsOnly, Category = "Input")
	UInputAction* ShowRankingAction;

	/* ---------- UI ---------- */
	// RankBoard
	UPROPERTY(EditDefaultsOnly, Category = "UI")
	TSubclassOf<UPTWRankingBoard> RankingBoardClass;
	UPROPERTY()
	TObjectPtr<UPTWRankingBoard> RankingBoard;

	/* ---------- 탄약 ---------- */
	void BindAmmoDelegate();
	void UnbindAmmoDelegate();
	void SyncAmmoUIOnce();
	void HandleAmmoChanged(int32 CurrentAmmo, int32 MaxAmmo);

	// 캐릭터에서 호출되는 콜백
	void HandleWeaponChanged(FGameplayTag NewWeaponTag, UPTWItemInstance* NewItemInstance);

	UPROPERTY()
	TObjectPtr<UPTWItemInstance> CurrentWeaponItem;
};
