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

USTRUCT(BlueprintType)
struct FSavedWeaponData
{
	GENERATED_BODY()
	
	TArray<FWeaponPair> WeaponArray;
};

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class PTW_API UPTWWeaponComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UPTWWeaponComponent();
	
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

public:
	/* 태그를 매개변수로 받아 무기교체 */
	UFUNCTION(BlueprintCallable, Category = "PTW|Weapon")
	void EquipWeaponByTag(FGameplayTag NewWeaponTag);

	/* 태그와 자동 몽타주 플레이를 실행받을지만 받아 무기 애니메이션 실행 */
	UFUNCTION(BlueprintCallable, Category = "PTW|Weapon")
	UAnimMontage* PlayWeaponMontages(FGameplayTag AnimTag, bool bAutoPlayCharacterMontage = false);

	/* WeaponTag에 따른 무기 소켓에 부착 */
	void AttachWeaponToSocket(APTWWeaponActor* NewWeapon1P, APTWWeaponActor* NewWeapon3P, FGameplayTag WeaponTag);

protected:
	UFUNCTION()
	void OnRep_CurrentWeaponTag(const FGameplayTag& OldTag);

private:
	float PlayCharacterMontage1P(UAnimMontage* MontageToPlay);

public:
	UPROPERTY(ReplicatedUsing = OnRep_CurrentWeaponTag, VisibleInstanceOnly, Category = "Weapon")
	FGameplayTag CurrentWeaponTag;

	UPROPERTY(VisibleInstanceOnly, Category = "PTW|Weapon")
	TMap<FGameplayTag, FWeaponPair> SpawnedWeapons;

	UPROPERTY(BlueprintReadOnly, Category = "PTW|Weapon", Replicated)
	TObjectPtr<APTWWeaponActor> CurrentWeapon;
};
