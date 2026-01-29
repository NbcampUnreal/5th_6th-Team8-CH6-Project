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
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
	virtual void OnRep_PlayerState() override;
	virtual void BeginSpectatingState() override;
	virtual void OnRep_Pawn() override;
	virtual void OnPossess(APawn* InPawn) override;
	virtual void OnUnPossess() override;

	/* HUD 초기화 */
	void TryInitializeHUD();
	// ASC 를 받아와서 HUD 에게 넘겨주는 함수이지만
	// 여기서 컨트롤러가 ASC 태그 변경에 델리게이트 바인드 하기도함
	
	/* 관전 시스템 함수 */
	void StartSpectating();
	UFUNCTION(NetMulticast, Reliable)
	void MulticastRPC_StartSpectating();
	UFUNCTION()
	void SpectateNextPlayer(APawn* InOldPawn, APawn* InNewPawn);
	UFUNCTION()
	void OnInputSpectateNext();
	
	/* KillLog 델리게이트 */
	FOnKillLog OnKillLog;
	
	/* 리스폰 타이머 핸들*/
	FTimerHandle RespawnTimerHandle;
protected:
	virtual void SetupInputComponent() override;
	virtual void PostSeamlessTravel() override;

	/* 랭킹보드 */
	void OnRankingPressed();
	void OnRankingReleased();
	void CreateRankingBoard();

	/* PauseMenu */
	void HandleMenuInput();

	/* 크로스헤어 */
	void OnCrosshairStateTagChanged(const FGameplayTag CallbackTag, int32 NewCount);
	void UpdateCrosshairVisibility();

	/* ---------- Input ---------- */
	// 랭킹보드 (Tab)
	UPROPERTY(EditDefaultsOnly, Category = "Input")
	UInputMappingContext* DefaultMappingContext;
	UPROPERTY(EditDefaultsOnly, Category = "Input")
	UInputAction* ShowRankingAction;

	UPROPERTY(EditDefaultsOnly, Category = "Input")
	UInputAction* SpectateNextAction;

	// PauseMenu (ESC)
	UPROPERTY(EditDefaultsOnly, Category = "Input")
	TObjectPtr<UInputAction> PauseMenuAction;
	
	/* ---------- UI ---------- */
	// 랭킹보드
	UPROPERTY(EditDefaultsOnly, Category = "UI")
	TSubclassOf<UPTWRankingBoard> RankingBoardClass;
	UPROPERTY()
	TObjectPtr<UPTWRankingBoard> RankingBoard;

	// PauseMenu
	UPROPERTY(EditDefaultsOnly, Category = "UI")
	TSubclassOf<UUserWidget> PauseMenuClass; 

	/* 탄약 */
	void BindAmmoDelegate();
	void UnbindAmmoDelegate();
	void SyncAmmoUIOnce();
	void HandleAmmoChanged(int32 CurrentAmmo, int32 MaxAmmo);
	// 캐릭터에서 호출되는 콜백
	void HandleWeaponChanged(FGameplayTag NewWeaponTag, UPTWItemInstance* NewItemInstance);

	UPROPERTY()
	TObjectPtr<UPTWItemInstance> CurrentWeaponItem;

	/* 플레이어 이름 */
	/* 닉네임 가시성 업데이트 로직 */
	void UpdateNameTagsVisibility();
	/* 가시성 설정 */
	UPROPERTY(EditDefaultsOnly, Category = "UI|NameTag")
	float NameTagMaxDistance = 1500.f;
	/* 가시성 체크 주기 */
	UPROPERTY(EditDefaultsOnly, Category = "UI|NameTag")
	float NameTagUpdateInterval = 0.1f; // 0.1초마다 체크
	UPROPERTY(EditDefaultsOnly, Category = "UI|NameTag")
	float NameTagMinScale = 0.4f; // 가장 멀리 있을 때의 최소 크기 (0.4)

	FTimerHandle NameTagTimerHandle;
};
