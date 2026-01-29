// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "PTWWeaponActor.h"
#include "PTWWeaponActor_Projectile.generated.h"

class UProjectileMovementComponent;

UCLASS()
class PTW_API APTWWeaponActor_Projectile : public APTWWeaponActor
{
	GENERATED_BODY()

public:
	APTWWeaponActor_Projectile();

protected:
	virtual void BeginPlay() override;
};
