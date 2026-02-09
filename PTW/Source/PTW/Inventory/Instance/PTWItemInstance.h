// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "../../Weapon/PTWWeaponActor.h"
#include "../../Weapon/PTWWeaponData.h"
#include "UObject/Object.h"
#include "PTWItemInstance.generated.h"

enum class EHitType : uint8;


class APTWWeaponActor;
class UPTWItemDefinition;
/**
 * 
 */
UCLASS()
class PTW_API UPTWItemInstance : public UObject
{
	GENERATED_BODY()
public:
	virtual bool IsSupportedForNetworking() const override {return true;}
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
public:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Replicated)
	TObjectPtr<UPTWItemDefinition> ItemDef;
};
