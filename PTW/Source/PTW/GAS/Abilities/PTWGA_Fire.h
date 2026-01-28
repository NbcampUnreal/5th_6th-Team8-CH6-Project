// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GAS/PTWGameplayAbility.h"
#include "PTWGA_Fire.generated.h"

class APTWProjectile;
class UPTWItemInstance;
/**
 * 
 */

USTRUCT(BlueprintType)
struct FPTWGameplayCueMakingInfo
{
	GENERATED_BODY();
	
public:
	UPROPERTY()
	UPTWItemInstance* ItemInstance;
	
	UPROPERTY()
	APTWPlayerCharacter* PlayerCharacter;	
};


UCLASS()
class PTW_API UPTWGA_Fire : public UPTWGameplayAbility
{
	GENERATED_BODY()
	
public:
	UPTWGA_Fire();
	
	virtual void InputReleased(const FGameplayAbilitySpecHandle Handle, 
		const FGameplayAbilityActorInfo* ActorInfo, 
		const FGameplayAbilityActivationInfo ActivationInfo) override;
	
	virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle, 
		const FGameplayAbilityActorInfo* ActorInfo, 
		const FGameplayAbilityActivationInfo ActivationInfo, 
		const FGameplayEventData* TriggerEventData) override;
	
	void StartFire(const FGameplayAbilitySpecHandle Handle, 
		const FGameplayAbilityActorInfo* ActorInfo,
		const FGameplayAbilityActivationInfo ActivationInfo);
	
	void AutoFire(const FGameplayAbilitySpecHandle Handle, 
		const FGameplayAbilityActorInfo* ActorInfo,
		const FGameplayAbilityActivationInfo ActivationInfo);
	
	void MakeGameplayCue(const FGameplayAbilitySpecHandle Handle, 
		const FGameplayAbilityActorInfo* ActorInfo,
		const FGameplayAbilityActivationInfo ActivationInfo,
		FPTWGameplayCueMakingInfo Infos);
	
	void StopFire();
	
protected:
	FTimerHandle AutoFireTimer;
	float FireRate;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Effect|Cue")
	TSubclassOf<UGameplayEffect> FireEffectClass;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Effect|Damage")
	TSubclassOf<UGameplayEffect> DamageGEClass;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Effect|Damage")
	TSubclassOf<UAttributeSet> WeaponAttributeClass;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Projectile")
	TSubclassOf<APTWProjectile> ProjectileClass;
	
protected:
	void PerformLineTrace(FHitResult& HitResult, APTWPlayerCharacter* PlayerCharacter);
	bool ValidateHitResult(FHitResult& HitResult);
	void ApplyDamageToTarget(const FGameplayAbilitySpecHandle Handle, 
		const FGameplayAbilityActorInfo* ActorInfo,
		const FGameplayAbilityActivationInfo ActivationInfo,
		const FGameplayAbilityTargetDataHandle& TargetData,
		float BaseDamage);
	
	virtual void EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled) override;
	
	UFUNCTION()
	void OnInputReleasedCallback(float TimeHold);
	
	void HitScanTypeFire(APTWPlayerCharacter* PC);
	
	void ProjectileTypeFire(APTWPlayerCharacter* PC, UPTWItemInstance* ItemInstance);
	
};
