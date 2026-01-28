// Fill out your copyright notice in the Description page of Project Settings.


#include "PTWWeaponActor_Projectile.h"

#include "GameFramework/ProjectileMovementComponent.h"


// Sets default values
APTWWeaponActor_Projectile::APTWWeaponActor_Projectile()
{
	PrimaryActorTick.bCanEverTick = false;
}

// Called when the game starts or when spawned
void APTWWeaponActor_Projectile::BeginPlay()
{
	Super::BeginPlay();
}


