// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "PTWWeaponActor.h"
#include "PTWWeaponActor_HitScan.generated.h"

UCLASS()
class PTW_API APTWWeaponActor_HitScan : public APTWWeaponActor
{
	GENERATED_BODY()

public:
	APTWWeaponActor_HitScan();

protected:
	virtual void BeginPlay() override;
	
};
