// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameplayCueNotify_Static.h"
#include "GC_EmptyAmmo.generated.h"

/**
 * 
 */
UCLASS()
class PTW_API UGC_EmptyAmmo : public UGameplayCueNotify_Static
{
	GENERATED_BODY()
	
protected:
	virtual bool OnExecute_Implementation(AActor* MyTarget, const FGameplayCueParameters& Parameters) const override;
	
	
protected:
	UPROPERTY(EditAnywhere)
	TObjectPtr<USoundCue> EmptyAmmoSound;
};
