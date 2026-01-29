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
class UWidgetComponent;

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

	virtual void HandleDeath(AActor* Attacker) override;
protected:
	//생성자
	virtual void BeginPlay() override;
	virtual void PossessedBy(AController* NewController) override;
	virtual void OnRep_PlayerState() override;
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;
	virtual void InitAbilityActorInfo() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

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

	UFUNCTION(BlueprintCallable, Category = "Weapon")
	void ApplyRecoil();
	UFUNCTION(BlueprintCallable, Category = "Weapon")
	float PlayMontage1P(UAnimMontage* MontageToPlay);

public:
	UPROPERTY(EditDefaultsOnly, Category = "Weapon")
	TMap<FGameplayTag, TSubclassOf<APTWWeaponActor>> WeaponClasses;
	UPROPERTY(ReplicatedUsing = OnRep_CurrentWeaponTag, VisibleInstanceOnly, Category = "Weapon")
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

	/* PlayerNameTag */
public:
	UWidgetComponent* GetNameTagWidget() const;
protected:
	/* 위젯에 닉네임 전달 */
	void UpdateNameTagText();

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "UI")
	TObjectPtr<UWidgetComponent> NameTagWidget;
	// 이름표 갱신 재시도를 위한 타이머 핸들
	FTimerHandle NameTagRetryTimer;
	
	UFUNCTION()
	void OnRep_CurrentWeaponTag(const FGameplayTag& OldTag);
	

};
