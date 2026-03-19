// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AbilitySystemInterface.h"
#include "GameFramework/PlayerState.h"
#include "PTWPlayerData.h"
#include "PTWPlayerRoundDataInterface.h"
#include "PTWPlayerState.generated.h"


class UPTWInventoryComponent;
class UPTWDeliveryAttributeSet;
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnPlayerDataChanged, const FPTWPlayerData&, NewData);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnPlayerRoundDataChanged, const FPTWPlayerRoundData&, NewData);

class UGameplayAbility;
class UGameplayEffect;
class UAbilitySystemComponent;
class UAttributeSet;
class UPTWWeaponAttributeSet;
class APTWShopNPC;

UCLASS()
class PTW_API APTWPlayerState : public APlayerState, public IAbilitySystemInterface, public IPTWPlayerRoundDataInterface
{
	GENERATED_BODY()
	
public:
	APTWPlayerState();

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	virtual UAbilitySystemComponent* GetAbilitySystemComponent() const override;
	UAttributeSet* GetAttributeSet() const { return AttributeSet; }
	
	FORCEINLINE UPTWInventoryComponent* GetInventoryComponent() const { return InventoryComponent; }

protected:
	/** Seamless Travel시 데이터 복사를 위해 사용되는 함수 */
	virtual void CopyProperties(APlayerState* PlayerState) override;
	
	UFUNCTION()
	void OnRep_CurrentPlayerData();
	
	UFUNCTION()
	void OnRep_PlayerRoundData();
	
	UFUNCTION()
	void OnRep_LobbyItemData();
public:
	UFUNCTION(BlueprintCallable, Category = "Data")
	void SetPlayerData(const FPTWPlayerData& NewData);
	UFUNCTION(BlueprintPure, Category = "Data")
	FPTWPlayerData GetPlayerData() const;

	UFUNCTION(BlueprintCallable, Category = "Data")
	void SetPlayerRoundData(const FPTWPlayerRoundData& NewData);
	UFUNCTION(BlueprintPure, Category = "Data")
	FPTWPlayerRoundData GetPlayerRoundData() const;

	UFUNCTION(BlueprintCallable, Category = "Data")
	void SetLobbyItemData(const FPTWLobbyItemData& NewData);
	FPTWLobbyItemData& GetLobbyItemData() {return LobbyItemData;}

	void SetMiniGameComponent(UActorComponent* NewMiniGameComponent);
	FORCEINLINE UActorComponent* GetMiniGameComponent(){return MiniGameComponent;}
	
	/** * 상점에 아이템 구매 요청
	 * @param ItemID : 구매할 아이템 ID (FName)
	 * @param Cost : 가격
	 */
	UFUNCTION(Server, Reliable, BlueprintCallable, Category = "Shop")
	void ServerRequestPurchase(APTWShopNPC* ShopNPC, FName ItemID, int32 Cost);

	UFUNCTION(Client, Reliable)
	void ClientPurchaseSuccess(APTWShopNPC* ShopNPC);

	UPROPERTY(BlueprintAssignable, Category = "Events")
	FOnPlayerDataChanged OnPlayerDataUpdated;

	UPROPERTY(BlueprintAssignable, Category = "Events")
	FOnPlayerRoundDataChanged OnPlayerRoundDataUpdated;

	//GAS 추가 GA,GE 변경함수
	UFUNCTION(BlueprintCallable, Category = "GAS|Injection")
	void InjectAbility(TSubclassOf<UGameplayAbility> AbilityClass);
	UFUNCTION(BlueprintCallable, Category = "GAS|Injection")
	void InjectEffect(TSubclassOf<UGameplayEffect> EffectClass);

	void ApplyAdditionalAbilities();
	void ApplyAdditionalEffects();

	void ApplyInvincible(float Duration);

	/* 타겟 POV 시스템 */
	// 타겟이 변경되었을 때 로컬 컨트롤러에 알림
	UFUNCTION()
	void OnRep_CurrentTargetPawn();
	// 현재 추격 중인 타겟 (복제 변수)
	UPROPERTY(ReplicatedUsing = OnRep_CurrentTargetPawn, VisibleAnywhere, BlueprintReadOnly, Category = "GTS|Target")
	TObjectPtr<APawn> CurrentTargetPawn;

	/* 심리스 트래블 시 이전 맵의 GAS 를 초기화하는 함수 */
	UFUNCTION(BlueprintCallable, Category = "GAS")
	void ClearGAS();

protected:
	UPROPERTY(VisibleAnywhere, Category = "GAS")
	TObjectPtr<UAbilitySystemComponent> AbilitySystemComponent;
	UPROPERTY()
	TObjectPtr<UAttributeSet> AttributeSet;
	UPROPERTY()
	TObjectPtr<UPTWWeaponAttributeSet> WeaponAttributeSet;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Inventory", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UPTWInventoryComponent> InventoryComponent;
	UPROPERTY(Replicated)
	TObjectPtr<UActorComponent> MiniGameComponent;
	
	UPROPERTY(ReplicatedUsing = OnRep_CurrentPlayerData, VisibleAnywhere, BlueprintReadOnly, Category = "Data")
	FPTWPlayerData CurrentPlayerData;

	UPROPERTY(ReplicatedUsing = OnRep_PlayerRoundData, VisibleAnywhere, BlueprintReadOnly, Category = "Data")
	FPTWPlayerRoundData PlayerRoundData;

	UPROPERTY(ReplicatedUsing = OnRep_LobbyItemData, VisibleAnywhere, BlueprintReadOnly, Category = "Data")
	FPTWLobbyItemData LobbyItemData;

	//GAS 추가 GA,GE 변수
	UPROPERTY(Transient)
	TArray<TSubclassOf<UGameplayAbility>> AdditionalAbilities;
	UPROPERTY(Transient)
	TArray<TSubclassOf<UGameplayEffect>> AdditionalEffects;

	//스폰 무적 이펙트
	UPROPERTY(EditAnywhere)
	TSubclassOf<UGameplayEffect> InvincibleEffectClass;
public:
	virtual void SetDeathOrder(int32 Order) override;
	virtual void SetTeamId(int32 TeamId) override;
	virtual int32 GetDeathOrder() const override {return PlayerRoundData.DeathOrder;} 
	virtual int32 GetTeamId() const override {return PlayerRoundData.TeamId;}
	
	virtual void AddKillCount(int32 AddKillCount = 1) override;
	virtual void AddDeathCount(int32 AddDeathCount = 1) override;
	virtual void AddScore(int32 AddScore) override;
	virtual void ResetRoundData() override;

	UFUNCTION(BlueprintCallable)
	void AddGold(int32 Amount);

	void ResetInventoryItemId();

	bool bIsReadyToPlay = false;
};
