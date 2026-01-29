// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GAS/PTWGameplayAbility.h"
#include "PTWGA_Reload.generated.h"

class UAbilityTask_PlayMontageAndWait;
class UAbilityTask_WaitGameplayEvent;

/**
 * 
 */
UCLASS()
class PTW_API UPTWGA_Reload : public UPTWGameplayAbility
{
	GENERATED_BODY()
	
public:
	UPTWGA_Reload();
	
	virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle, 
		const FGameplayAbilityActorInfo* ActorInfo, 
		const FGameplayAbilityActivationInfo ActivationInfo, 
		const FGameplayEventData* TriggerEventData) override;
	
protected:
	/** 몽타주가 정상 종료되었을 때 (BlendOut 포함) */
	UFUNCTION()
	void OnMontageCompleted();
	/** 몽타주가 취소/중단되었을 때 */
	UFUNCTION()
	void OnMontageCancelled();
	/** AnimNotify로부터 게임플레이 이벤트를 수신했을 때 (여기서 GE 적용) */
	UFUNCTION()
	void OnGameplayEventReceived(FGameplayEventData Payload);

public:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Effect")
	TSubclassOf<UGameplayEffect> ReloadEffectClass;

	/** WeaponData에서 검색할 몽타주 태그 (기본값: Weapon.Anim.Reload) */
	UPROPERTY(EditDefaultsOnly, Category = "Config")
	FGameplayTag ReloadAnimTag;
};
