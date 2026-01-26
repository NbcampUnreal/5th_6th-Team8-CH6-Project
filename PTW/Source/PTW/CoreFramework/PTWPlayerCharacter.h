// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "PTWBaseCharacter.h"
#include "InputActionValue.h"
#include "PTWInputConfig.h"
#include "PTWPlayerCharacter.generated.h"

class APTWPlayerState;
class UPTWItemDefinition;
class UCameraComponent;
class UInputMappingContext;
class UInputAction;
class UPTWInventoryComponent;
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
class PTW_API APTWPlayerCharacter : public APTWBaseCharacter
{
	GENERATED_BODY()
	
public:
	APTWPlayerCharacter();

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

protected:
	//생성자
	virtual void BeginPlay() override;
	virtual void PossessedBy(AController* NewController) override;
	virtual void OnRep_PlayerState() override;
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;
	virtual void InitAbilityActorInfo() override;

protected:
	void Move(const FInputActionValue& Value);
	void Look(const FInputActionValue& Value);
	void Input_AbilityInputTagPressed(FGameplayTag InputTag);
	void Input_AbilityInputTagReleased(FGameplayTag InputTag);

public:
	UFUNCTION(BlueprintCallable, Category = "Weapon")
	void EquipWeaponByTag(FGameplayTag NewWeaponTag);
	UFUNCTION(BlueprintCallable, Category = "Weapon")
	void OnRep_CurrentWeapon(APTWWeaponActor* OldWeapon);
	UFUNCTION(BlueprintCallable, Category = "Weapon")
	APTWWeaponActor* GetCurrentWeapon() const { return CurrentWeapon; }

	UFUNCTION(BlueprintPure, Category = "Mesh")
	FORCEINLINE USkeletalMeshComponent* GetMesh1P() const { return Mesh1P; }
	UFUNCTION(BlueprintPure, Category = "Mesh")
	FORCEINLINE USkeletalMeshComponent* GetMesh3P() const { return GetMesh(); }

	void AttachWeaponToSocket(APTWWeaponActor* NewWeapon1P, APTWWeaponActor* NewWeapon3P, FGameplayTag WeaponTag);

public:
	UPROPERTY(EditDefaultsOnly, Category = "Weapon")
	TMap<FGameplayTag, TSubclassOf<APTWWeaponActor>> WeaponClasses;
	UPROPERTY(Replicated, VisibleInstanceOnly, Category = "Weapon")
	FGameplayTag CurrentWeaponTag;
	UPROPERTY(VisibleInstanceOnly, Category = "Weapon")
	TMap<FGameplayTag, FWeaponPair> SpawnedWeapons;
	UPROPERTY(BlueprintReadOnly, Category = "Weapon", ReplicatedUsing = OnRep_CurrentWeapon)
	APTWWeaponActor* CurrentWeapon;

protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Camera")
	TObjectPtr<UCameraComponent> PlayerCamera;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Mesh, meta = (AllowPrivateAccess = "true"))
	TObjectPtr<USkeletalMeshComponent> Mesh1P;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Inventory", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UPTWInventoryComponent> InventoryComponent;

	UPROPERTY(EditDefaultsOnly, Category = "Input")
	TObjectPtr<UPTWInputConfig> InputConfig;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
	TObjectPtr<UInputMappingContext> DefaultMappingContext;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
	TObjectPtr<UInputAction> MoveAction;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
	TObjectPtr<UInputAction> LookAction;

	//FIXME : 테스트 용도
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Temp")
	TObjectPtr<UPTWItemDefinition> ItemDef;
	
public:
	FORCEINLINE UCameraComponent* GetPlayerCamera() const { return PlayerCamera; }
	FORCEINLINE UPTWInventoryComponent* GetInventoryComponent() const { return InventoryComponent; }
};
