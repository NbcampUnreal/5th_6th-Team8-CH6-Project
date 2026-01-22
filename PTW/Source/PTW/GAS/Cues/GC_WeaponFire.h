// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameplayCueNotify_Static.h"
#include "GC_WeaponFire.generated.h"

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
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "VFX")
	TObjectPtr<UNiagaraSystem> FireVFX;
};
