// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "GameplayTagContainer.h"
#include "PTWWeaponComponent.generated.h"

class APTWWeaponActor;
class UAnimMontage;

USTRUCT(BlueprintType)
struct FWeaponPair
{
	GENERATED_BODY()
public:
	UPROPERTY(EditDefaultsOnly)
	TObjectPtr<APTWWeaponActor> Weapon1P;
	UPROPERTY(EditDefaultsOnly)
	TObjectPtr<APTWWeaponActor> Weapon3P;
};

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class PTW_API UPTWWeaponComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UPTWWeaponComponent();
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

public:
	UFUNCTION(BlueprintCallable, Category = "Weapon")
	void EquipWeaponByTag(FGameplayTag NewWeaponTag);
	void AttachWeaponToSocket(APTWWeaponActor* NewWeapon1P, APTWWeaponActor* NewWeapon3P, FGameplayTag WeaponTag);
	UFUNCTION(BlueprintCallable, Category = "Weapon")
	void ApplyRecoil();
	UFUNCTION(BlueprintCallable, Category = "Weapon")
	float PlayMontage1P(UAnimMontage* MontageToPlay);


protected:
	UFUNCTION()
	void OnRep_CurrentWeaponTag(const FGameplayTag& OldTag);
	UFUNCTION()
	void OnRep_CurrentWeapon(APTWWeaponActor* OldWeapon);

public:
	UPROPERTY(ReplicatedUsing = OnRep_CurrentWeaponTag, VisibleInstanceOnly, Category = "Weapon")
	FGameplayTag CurrentWeaponTag;
	UPROPERTY(VisibleInstanceOnly, Category = "Weapon")
	TMap<FGameplayTag, FWeaponPair> SpawnedWeapons;
	UPROPERTY(BlueprintReadOnly, Category = "Weapon", ReplicatedUsing = OnRep_CurrentWeapon)
	TObjectPtr<APTWWeaponActor> CurrentWeapon;
};
