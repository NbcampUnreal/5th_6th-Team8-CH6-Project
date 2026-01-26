// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "AbilitySystemInterface.h"
#include "GameplayTagContainer.h"
#include "PTWBaseCharacter.generated.h"

class UAbilitySystemComponent;
class UAttributeSet;
class UGameplayAbility;
class UGameplayEffect;
class APTWWeaponActor;

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


UCLASS()
class PTW_API APTWBaseCharacter : public ACharacter, public IAbilitySystemInterface
{
	GENERATED_BODY()

public:
	//생성자
	APTWBaseCharacter();
	
	virtual void BeginPlay() override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	virtual UAbilitySystemComponent* GetAbilitySystemComponent() const override;

	UFUNCTION(BlueprintPure)
	bool IsDead() const;
	
protected:

	virtual void InitAbilityActorInfo();

	void GiveDefaultAbilities();
	void ApplyDefaultEffects();

public:
	UFUNCTION(BlueprintCallable)
	void EquipWeaponByTag(FGameplayTag NewWeaponTag);
	
	virtual void HandleDeath(AActor* Attacker);
	
	UFUNCTION(BlueprintCallable)
	void OnRep_CurrentWeapon(APTWWeaponActor* OldWeapon);

protected:


public:
	UPROPERTY(EditDefaultsOnly, Category = "Weapon")
	TMap<FGameplayTag, TSubclassOf<APTWWeaponActor>> WeaponClasses;
	UPROPERTY(Replicated, VisibleInstanceOnly, Category = "Weapon")
	FGameplayTag CurrentWeaponTag;
	// UPROPERTY(VisibleInstanceOnly, Category = "Weapon")
	// TMap<FGameplayTag, APTWWeaponActor*> SpawnedWeapons;
	
	UPROPERTY(VisibleInstanceOnly, Category = "Weapon")
	TMap<FGameplayTag, FWeaponPair> SpawnedWeapons;
	
	//26.01.26 수정됨(현정석)
	UPROPERTY(BlueprintReadOnly, Category = "Weapon", ReplicatedUsing= OnRep_CurrentWeapon)
	APTWWeaponActor* CurrentWeapon;
	
	FGameplayTag DeadTag;


protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "GAS")
	TObjectPtr<UAbilitySystemComponent> AbilitySystemComponent;
	UPROPERTY()
	TObjectPtr<UAttributeSet> AttributeSet;

	UPROPERTY(EditAnywhere, Category = "GAS|Default")
	TArray<TSubclassOf<UGameplayAbility>> DefaultAbilities;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "GAS|Default")
	TArray<TSubclassOf<UGameplayEffect>> DefaultEffects;
	
};
