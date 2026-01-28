// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AbilitySystemInterface.h"
#include "GameFramework/PlayerState.h"
#include "PTWPlayerData.h"
#include "PTWPlayerState.generated.h"

class UPTWWeaponAttributeSet;
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnPlayerDataChanged, const FPTWPlayerData&, NewData);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnPlayerRoundDataChanged, const FPTWPlayerRoundData&, NewData);

class UAbilitySystemComponent;
class UAttributeSet;

UCLASS()
class PTW_API APTWPlayerState : public APlayerState, public IAbilitySystemInterface
{
	GENERATED_BODY()
	
public:
	APTWPlayerState();

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	virtual UAbilitySystemComponent* GetAbilitySystemComponent() const override;
	UAttributeSet* GetAttributeSet() const { return AttributeSet; }

protected:
	UFUNCTION()
	void OnRep_CurrentPlayerData();
	
	UFUNCTION()
	void OnRep_PlayerRoundData();
public:
	UFUNCTION(BlueprintCallable, Category = "Data")
	void SetPlayerData(const FPTWPlayerData& NewData);
	UFUNCTION(BlueprintPure, Category = "Data")
	FPTWPlayerData GetPlayerData() const;

	UFUNCTION(BlueprintCallable, Category = "Data")
	void SetPlayerRoundData(const FPTWPlayerRoundData& NewData);
	UFUNCTION(BlueprintPure, Category = "Data")
	FPTWPlayerRoundData GetPlayerRoundData() const;

	UPROPERTY(BlueprintAssignable, Category = "Events")
	FOnPlayerDataChanged OnPlayerDataUpdated;

	UPROPERTY(BlueprintAssignable, Category = "Events")
	FOnPlayerRoundDataChanged OnPlayerRoundDataUpdated;

protected:
	UPROPERTY(VisibleAnywhere, Category = "GAS")
	TObjectPtr<UAbilitySystemComponent> AbilitySystemComponent;
	UPROPERTY()
	TObjectPtr<UAttributeSet> AttributeSet;
	UPROPERTY()
	TObjectPtr<UPTWWeaponAttributeSet> WeaponAttributeSet;

	UPROPERTY(ReplicatedUsing = OnRep_CurrentPlayerData, VisibleAnywhere, BlueprintReadOnly, Category = "Data")
	FPTWPlayerData CurrentPlayerData;

	UPROPERTY(ReplicatedUsing = OnRep_PlayerRoundData, VisibleAnywhere, BlueprintReadOnly, Category = "Data")
	FPTWPlayerRoundData PlayerRoundData;

public:
	void AddKillCount(int32 AddKillCount = 1);
	void AddDeathCount(int32 AddDeathCount = 1);
	void AddScore(int32 AddScore);

	void ResetPlayerRoundData();
};
