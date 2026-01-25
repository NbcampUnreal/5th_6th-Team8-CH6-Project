// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AbilitySystemInterface.h"
#include "GameFramework/PlayerState.h"
#include "PTWPlayerData.h"
#include "PTWPlayerState.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnPlayerDataChanged, const FPTWPlayerData&, NewData);

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

public:
	UFUNCTION(BlueprintCallable, Category = "Data")
	void SetPlayerData(const FPTWPlayerData& NewData);
	UFUNCTION(BlueprintPure, Category = "Data")
	FPTWPlayerData GetPlayerData() const;

	UPROPERTY(BlueprintAssignable, Category = "Events")
	FOnPlayerDataChanged OnPlayerDataUpdated;

protected:
	UPROPERTY(VisibleAnywhere, Category = "GAS")
	TObjectPtr<UAbilitySystemComponent> AbilitySystemComponent;
	UPROPERTY()
	TObjectPtr<UAttributeSet> AttributeSet;

	UPROPERTY(ReplicatedUsing = OnRep_CurrentPlayerData, VisibleAnywhere, BlueprintReadOnly, Category = "Data")
	FPTWPlayerData CurrentPlayerData;
};
