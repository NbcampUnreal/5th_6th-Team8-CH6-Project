// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "PTWItemInstance.h"
#include "PTWWeaponInstance.generated.h"

class APTWPlayerCharacter;
DECLARE_MULTICAST_DELEGATE_TwoParams(FOnAmmoChangedSignature, int32 /*CurrentAmmo*/, int32 /*MaxAmmo*/);

/**
 * 
 */
UCLASS()
class PTW_API UPTWWeaponInstance : public UPTWItemInstance
{
	GENERATED_BODY()
	
public:
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	
	UFUNCTION()
	void OnRep_CurrentAmmo();
	
	UFUNCTION()
	void OnRep_SpawnedWeapon();
	
	UFUNCTION()
	void OnRep_SpawnedWeapon3P();
	
	FORCEINLINE EHitType GetWeaponHitType() const {return SpawnedWeapon1P->GetWeaponData()->HitType;}
	FORCEINLINE UPTWWeaponData* GetWeaponData() const {return SpawnedWeapon1P->GetWeaponData();}
	
	void DestroySpawnedActors();
	
	/* UI 연동 */
	UFUNCTION(BlueprintCallable, Category = "Weapon")
	void SetCurrentAmmo(int32 NewAmmo);
	UFUNCTION(BlueprintPure, Category = "Weapon")
	int32 GetMaxAmmo();
	
	APTWPlayerCharacter* GetItemInstanceOwner();
	
public:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, ReplicatedUsing = OnRep_CurrentAmmo)
	int32 CurrentAmmo;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, ReplicatedUsing = OnRep_SpawnedWeapon)
	TObjectPtr<APTWWeaponActor> SpawnedWeapon1P;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, ReplicatedUsing = OnRep_SpawnedWeapon3P)
	TObjectPtr<APTWWeaponActor> SpawnedWeapon3P;
	
	// UI 연동, PlayerController 에서 바인딩할 델리게이트
	FOnAmmoChangedSignature OnAmmoChanged;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Replicated)
	bool bAlreadyUsing = false;
};
