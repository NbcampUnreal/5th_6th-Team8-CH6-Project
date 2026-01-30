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
	/* HUD 초기화 */
	void TryInitializeHUD();
	void StartRetryTimer();
	void StopRetryTimer();
	
	/*  ASC Delegate 바인딩 */
	void BindASCDelegates(UAbilitySystemComponent* ASC);
	void UnbindASCDelegates(UAbilitySystemComponent* ASC);

	/* 관전 시스템 함수 */
	void StartSpectating();
	UFUNCTION(NetMulticast, Reliable)
	void MulticastRPC_StartSpectating();
	UFUNCTION()
	void SpectateNextPlayer(APawn* InOldPawn, APawn* InNewPawn);
	APawn* FindNextSpectatorTarget(APawn* InNewPawn);
	void SetSpectatorTarget(APawn* NewViewTarget);
	UFUNCTION()
	void OnInputSpectateNext();
	
	/* 데미지 인디케이터 */
	UFUNCTION(Client, Reliable)
	void ClientRPC_ShowDamageIndicator(FVector DamageCauserLocation);

protected:
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
	virtual void OnRep_PlayerState() override;
	virtual void BeginSpectatingState() override;
	virtual void OnRep_Pawn() override;
	virtual void OnPossess(APawn* InPawn) override;
	virtual void OnUnPossess() override;
	virtual void SetViewTarget(AActor* NewViewTarget, 
		FViewTargetTransitionParams TransitionParams = FViewTargetTransitionParams()) override;
	
	void SetOwnerNoSeeRecursive(USceneComponent* InParentComponent, bool bNewOwnerNoSee);
	void SetSetOnlyOwnerSeeRecursive(USceneComponent* InParentComponent, bool bNewOnlyOwnerSee);
	
	virtual void SetupInputComponent() override;
	virtual void PostSeamlessTravel() override;

	/* 랭킹보드 */
	void OnRankingPressed();
	void OnRankingReleased();
	void CreateRankingBoard();

	/* PauseMenu */
	void HandleMenuInput();

	/* 크로스헤어 */
	void OnCrosshairStateTagChanged(const FGameplayTag Tag, int32 NewCount);
	void UpdateCrosshairVisibility();
	
	/* 플레이어 이름 */
	/* 닉네임 가시성 업데이트 로직 */
	void UpdateNameTagsVisibility();
	
public:
	/* KillLog 델리게이트 */
	FOnKillLog OnKillLog;
	
	/* 리스폰 타이머 핸들*/
	FTimerHandle RespawnTimerHandle;
	
protected:
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
	
	/* 가시성 설정 */
	UPROPERTY(EditDefaultsOnly, Category = "UI|NameTag")
	float NameTagMaxDistance = 1500.f;
	/* 가시성 체크 주기 */
	UPROPERTY(EditDefaultsOnly, Category = "UI|NameTag")
	float NameTagUpdateInterval = 0.1f; // 0.1초마다 체크
	UPROPERTY(EditDefaultsOnly, Category = "UI|NameTag")
	float NameTagMinScale = 0.4f; // 가장 멀리 있을 때의 최소 크기 (0.4)

	FTimerHandle NameTagTimerHandle;

	// HUD 초기화용
	FTimerHandle HUDInitTimerHandle;
	
	// GameplayTag Delegate Handles
	FDelegateHandle EquipTagHandle;
	FDelegateHandle SprintTagHandle;

	// 캐싱된 태그 (매번 Request 하지 않기 위함)
	FGameplayTag EquipTag;
	FGameplayTag SprintTag;
};
