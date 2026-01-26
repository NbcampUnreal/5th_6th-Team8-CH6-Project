// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Object.h"
#include "PTWItemInstance.generated.h"

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
	
	UFUNCTION()
	void OnRep_CurrentAmmo();
	
	UFUNCTION()
	void OnRep_SpawnedWeapon();
	
	UFUNCTION()
	void OnRep_SpawnedWeapon3P();
	
public:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Replicated)
	TObjectPtr<UPTWItemDefinition> ItemDef;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, ReplicatedUsing = OnRep_CurrentAmmo)
	int32 CurrentAmmo;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, ReplicatedUsing = OnRep_SpawnedWeapon)
	TObjectPtr<APTWWeaponActor> SpawnedWeapon1P;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, ReplicatedUsing = OnRep_SpawnedWeapon3P)
	TObjectPtr<APTWWeaponActor> SpawnedWeapon3P;
	
};
