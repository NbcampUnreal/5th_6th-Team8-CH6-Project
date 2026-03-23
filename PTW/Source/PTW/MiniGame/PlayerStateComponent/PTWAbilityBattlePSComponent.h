// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "PTWAbilityBattlePSComponent.generated.h"
DECLARE_MULTICAST_DELEGATE_TwoParams(FOnDraftChargedTimeChanged, float, float);
DECLARE_MULTICAST_DELEGATE_OneParam(FOnChangedChargeCount, int32);

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class PTW_API UPTWAbilityBattlePSComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UPTWAbilityBattlePSComponent();

	virtual void GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const override;
	
	void AddDraftCharges();
	void DecreaseDraftCharges();
	
	void SetCurrentDraft(const TArray<FName>& NewDraft);
	void ResetCurrentDraft();
	
	void UpdateChargeRemainTime(float DecreaseTimer);
	
	
	UPROPERTY()
	float MaxChargeTime = 25.f;
	
	UPROPERTY(ReplicatedUsing = OnRep_ChargeRemainTime)
	float ChargeRemainTime = MaxChargeTime;

	UFUNCTION()
	void OnRep_ChargeRemainTime();
	
	UPROPERTY(ReplicatedUsing = OnRep_ChangeChargeCount,VisibleAnywhere)
	int32 DraftChargeCount = 1;

	UFUNCTION()
	void OnRep_ChangeChargeCount();
	
	UPROPERTY(VisibleAnywhere)
	TArray<FName> CurrentDraft;

	UPROPERTY(Replicated)
	bool bFirstDraftCompleted = false;
	
	FOnDraftChargedTimeChanged OnDraftChargedTimeChanged;
	FOnChangedChargeCount OnChangedChargeCount;

	FORCEINLINE TArray<FName> GetCurrentDraft() {return CurrentDraft;}
	
};
