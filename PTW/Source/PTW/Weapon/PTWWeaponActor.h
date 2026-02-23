// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "PTWWeaponTypes.h"
#include "PTWWeaponActor.generated.h"

class USphereComponent;
class UPTWWeaponInstance;
class UPTWWeaponData;

UCLASS()
class PTW_API APTWWeaponActor : public AActor
{
	GENERATED_BODY()

public:
	APTWWeaponActor();
	
	UFUNCTION(BlueprintPure)
	FORCEINLINE USceneComponent* GetMuzzleComponent() const { return MuzzleSocket; }
	
	UFUNCTION(BlueprintPure)
	FORCEINLINE UPTWWeaponData* GetWeaponData() const {return WeaponData;}
	
	UFUNCTION(BlueprintPure)
	FORCEINLINE USkeletalMeshComponent* GetWeaponMesh() const {return WeaponMesh;}
	
	void ApplyVisualPerspective();
	
	virtual void GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const override;
	
	void SetFirstPersonMode(bool bIsFirstPerson);
	
	FORCEINLINE bool IsFirstPersonMode() const {return bIsFirstPersonWeapon;}

	UFUNCTION(BlueprintCallable, Category = "Reload")
	void HandleReloadEvent(EReloadEventAction ActionType);

	float PlayWeaponMontage(UAnimMontage* MontageToPlay);
	
	void SetWeaponItemInstance(UPTWWeaponInstance* ItemInstance);
	
	FORCEINLINE UPTWWeaponInstance* GetWeaponItemInstance() const {return WeaponItemInstance;}
	
	FORCEINLINE bool GetIsDrop() const {return bIsDrop;}
	
	FORCEINLINE void SetIsDrop(bool bHasDrop) { bIsDrop = bHasDrop;}
	
protected:
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
	virtual bool ReplicateSubobjects(class UActorChannel* Channel, class FOutBunch* Bunch, FReplicationFlags* RepFlags) override;
	
	UFUNCTION()
	void OnRep_IsFirstPersonWeapon();
	
	UFUNCTION()
	void OnRep_IsDrop();
	
	//재장전 함수
	void DropMag();
	void GrabMag();
	void InsertMag();
	
	void SetDroppingMode();

protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Weapon")
	TObjectPtr<USceneComponent> RootScene;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Weapon")
	TObjectPtr<USceneComponent> MuzzleSocket;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Weapon")
	TObjectPtr<USkeletalMeshComponent> WeaponMesh;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Weapon")
	TObjectPtr<USphereComponent> SphereComponent;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Weapon|Data")
	TObjectPtr<UPTWWeaponData> WeaponData;	
	
	UPROPERTY(ReplicatedUsing = OnRep_IsFirstPersonWeapon)
	bool bIsFirstPersonWeapon = false;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, ReplicatedUsing = OnRep_IsDrop)
	bool bIsDrop = false;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Replicated)
	TObjectPtr<UPTWWeaponInstance> WeaponItemInstance;
	
	// 재장전 관련 변수
	UPROPERTY(EditDefaultsOnly, Category = "Reload")
	TSubclassOf<AActor> MagazineClass;
	UPROPERTY(EditDefaultsOnly, Category = "Reload")
	TSubclassOf<AActor> EmptyMagazineClass;
	UPROPERTY(EditDefaultsOnly, Category = "Reload")
	FName MagBoneName = FName("b_weapon_mag");
	UPROPERTY(EditDefaultsOnly, Category = "Reload")
	FName HandSocketName = FName("Mag_Socket");
	UPROPERTY()
	TObjectPtr<AActor> CurrentFakeMag;
};
