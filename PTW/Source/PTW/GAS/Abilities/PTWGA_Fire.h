// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GAS/PTWGameplayAbility.h"
#include "PTWGA_Fire.generated.h"

class UPTWWeaponInstance;
class APTWWeaponActor;
class APTWProjectile;
/**
 * 
 */

USTRUCT(BlueprintType)
struct FPTWGameplayCueMakingInfo
{
	GENERATED_BODY();
	
public:
	UPROPERTY()
	APTWWeaponActor* Weapon1P;
	
	UPROPERTY()
	APTWPlayerCharacter* PlayerCharacter;	
};

USTRUCT(BlueprintType)
struct FPTWFireConext
{
	GENERATED_BODY();
	
public:
	UPROPERTY()
	APTWPlayerCharacter* PC = nullptr;
	
	UPROPERTY()
	UAbilitySystemComponent* ASC = nullptr;
	
	UPROPERTY()
	UPTWWeaponInstance* WeaponInst = nullptr;
	
	bool IsValid() const {return PC && ASC && WeaponInst;}
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
	
	virtual void StartFire();
	
	virtual void AutoFire();
	
	virtual void MakeGameplayCue(FPTWGameplayCueMakingInfo Infos);
	
	virtual void StopFire();
	
	virtual FPTWFireConext GetFireContext() const;
	
protected:
	FTimerHandle AutoFireTimer;
	float FireRate;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Effect|Cue")
	TSubclassOf<UGameplayEffect> FireEffectClass;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Effect|Damage")
	TSubclassOf<UGameplayEffect> DamageGEClass;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Projectile")
	TSubclassOf<UGameplayEffect> HeadShotEffectClass;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Effect|Damage")
	TSubclassOf<UAttributeSet> WeaponAttributeClass;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Projectile")
	TSubclassOf<APTWProjectile> ProjectileClass;
	
protected:
	virtual void PerformLineTrace(FHitResult& HitResult, APTWPlayerCharacter* PlayerCharacter);
	virtual bool ValidateHitResult(FHitResult& HitResult);
	virtual void ApplyDamageToTarget(const FGameplayAbilityTargetDataHandle& TargetData, float BaseDamage);
	
	virtual void EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled) override;
	
	UFUNCTION()
	virtual void OnInputReleasedCallback(float TimeHold);
	
	virtual void ProjectileTypeFire(APTWPlayerCharacter* PC, UPTWWeaponInstance* ItemInstance);
	
	virtual void HandleHitScan(const FPTWFireConext Context);
	
	virtual void ExecuteHitImpactCue(const FHitResult& HitResult);
	
	virtual bool CheckingTag(UAbilitySystemComponent* ASC);
	
private:
	float MaxRange = 5000.0f;
	
	UPROPERTY(EditDefaultsOnly, Category = "IgnoreTag")
	FGameplayTag IgnoreTag;
};
