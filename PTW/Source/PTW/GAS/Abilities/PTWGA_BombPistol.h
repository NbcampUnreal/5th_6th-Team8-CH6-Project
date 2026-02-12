// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "PTWGA_Fire.h"
#include "PTWGA_BombPistol.generated.h"

class UPTWItemDefinition;
/**
 * 
 */
UCLASS()
class PTW_API UPTWGA_BombPistol : public UPTWGA_Fire
{
	GENERATED_BODY()
	
protected:
	virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData) override;
	
	virtual void ApplyDamageToTarget(
		const FGameplayAbilityTargetDataHandle& TargetData,
		float BaseDamage) override;
	
	void AttachBombToTarget(AActor* Bomb, AActor* Target);
	void ProcessItemTransfer(AActor* Source, AActor* Target);
	void GiveItemAndEquip(APTWPlayerCharacter* TargetPC);
	void RemoveSourceWeapon(APTWPlayerCharacter* SourcePC);

protected:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Special Effects")
	TSubclassOf<UGameplayEffect> BombPistolEffect;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Special Effects")
	TObjectPtr<UPTWItemDefinition> ItemDef;
	
protected:
	bool ConsumeReflectShieldIfAny(AActor* TargetActor) const;
};
