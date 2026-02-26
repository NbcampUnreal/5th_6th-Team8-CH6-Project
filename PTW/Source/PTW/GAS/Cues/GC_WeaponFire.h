// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameplayCueNotify_Static.h"
#include "GC_WeaponFire.generated.h"

class UPTWWeaponSoundTable;
class APTWWeaponActor;
class UNiagaraSystem;
/**
 * 
 */
UCLASS()
class PTW_API UGC_WeaponFire : public UGameplayCueNotify_Static
{
	GENERATED_BODY()
	
public:
	virtual bool OnExecute_Implementation(AActor* MyTarget, 
		const FGameplayCueParameters& Parameters) const override;
	
	void SpawnCasing(const APTWWeaponActor* TargetWeapon) const;
	
public:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "VFX")
	TObjectPtr<UNiagaraSystem> FireVFX;
	
	// UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "SFX")
	// TArray<TObjectPtr<USoundCue>> FireSFX;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Data")
	TObjectPtr<UPTWWeaponSoundTable> WeaponSoundTable;
};
