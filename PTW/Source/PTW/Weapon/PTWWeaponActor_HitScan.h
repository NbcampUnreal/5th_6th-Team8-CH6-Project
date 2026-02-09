// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "../Weapon/PTWWeaponActor.h"
#include "PTWWeaponActor_HitScan.generated.h"

class APTWWeaponCasting;

UCLASS()
class PTW_API APTWWeaponActor_HitScan : public APTWWeaponActor
{
	GENERATED_BODY()

public:
	APTWWeaponActor_HitScan();

	FORCEINLINE USceneComponent* GetCasingMuzzle() const {return CastingSocket;}
	
	void SpawnCastingActor() const;
protected:
	virtual void BeginPlay() override;

protected:
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	TObjectPtr<USceneComponent> CastingSocket;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	TSubclassOf<APTWWeaponCasting> CastingClass;
};
