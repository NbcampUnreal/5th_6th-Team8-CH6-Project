// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "PTWWeaponActor.generated.h"

class UPTWWeaponData;

UCLASS()
class PTW_API APTWWeaponActor : public AActor
{
	GENERATED_BODY()

public:
	APTWWeaponActor();

protected:
	virtual void BeginPlay() override;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "")
	TObjectPtr<USceneComponent> MuzzleSocket;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "")
	TObjectPtr<UStaticMeshComponent> WeaponMesh;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "")
	TObjectPtr<UPTWWeaponData> WeaponData;	
};
