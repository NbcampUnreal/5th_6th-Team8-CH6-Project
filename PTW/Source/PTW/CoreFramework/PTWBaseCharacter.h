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

UCLASS()
class PTW_API APTWBaseCharacter : public ACharacter, public IAbilitySystemInterface
{
	GENERATED_BODY()

public:
	//생성자
	APTWBaseCharacter();
	
	virtual void BeginPlay() override;
	virtual UAbilitySystemComponent* GetAbilitySystemComponent() const override;

protected:

	virtual void InitAbilityActorInfo();

	void GiveDefaultAbilities();
	void ApplyDefaultEffects();

public:
	UPROPERTY(EditDefaultsOnly, Category = "Weapon")
	TMap<FGameplayTag, TSubclassOf<APTWWeaponActor>> WeaponClasses;
	UPROPERTY(VisibleInstanceOnly, Category = "Weapon")
	FGameplayTag CurrentWeaponTag;
	UPROPERTY(VisibleInstanceOnly, Category = "Weapon")
	TMap<FGameplayTag, APTWWeaponActor*> SpawnedWeapons;
	UPROPERTY(BlueprintReadOnly, Category = "Weapon")
	APTWWeaponActor* CurrentWeapon;

	UFUNCTION(BlueprintCallable)
	void EquipWeaponByTag(FGameplayTag NewWeaponTag);
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
