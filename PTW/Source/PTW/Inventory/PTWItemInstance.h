// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "PTWWeaponActor.h"
#include "PTWWeaponData.h"
#include "UObject/Object.h"
#include "PTWItemInstance.generated.h"

enum class EHitType : uint8;
DECLARE_MULTICAST_DELEGATE_TwoParams(FOnAmmoChangedSignature, int32 /*CurrentAmmo*/, int32 /*MaxAmmo*/);

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
	
	FORCEINLINE EHitType GetWeaponHitType() const {return SpawnedWeapon1P->GetWeaponData()->HitType;}
	FORCEINLINE UPTWWeaponData* GetWeaponData() const {return SpawnedWeapon1P->GetWeaponData();}
	
public:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Replicated)
	TObjectPtr<UPTWItemDefinition> ItemDef;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, ReplicatedUsing = OnRep_CurrentAmmo)
	int32 CurrentAmmo;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, ReplicatedUsing = OnRep_SpawnedWeapon)
	TObjectPtr<APTWWeaponActor> SpawnedWeapon1P;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, ReplicatedUsing = OnRep_SpawnedWeapon3P)
	TObjectPtr<APTWWeaponActor> SpawnedWeapon3P;
	
	// UI 연동, PlayerController 에서 바인딩할 델리게이트
	FOnAmmoChangedSignature OnAmmoChanged;
	
	/* UI 연동 */
	UFUNCTION(BlueprintCallable, Category = "Weapon")
	void SetCurrentAmmo(int32 NewAmmo);
	UFUNCTION(BlueprintPure, Category = "Weapon")
	int32 GetMaxAmmo();
};
