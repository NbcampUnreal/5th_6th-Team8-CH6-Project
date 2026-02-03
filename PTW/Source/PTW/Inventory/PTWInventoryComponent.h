// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "PTWInventoryComponent.generated.h"


struct FGameplayTag;
class UPTWItemInstance;
class APTWWeaponActor;
class UPTWItemDefinition;
class UAbilitySystemComponent;
class UPTWWeaponInstance;




UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class PTW_API UPTWInventoryComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	// Sets default values for this component's properties
	UPTWInventoryComponent();
	
	//FIXME : 파리미터 임시 추가(WeaponActor)
	void AddItem(TObjectPtr<UPTWItemInstance>);
	void SwapWeapon(int32 SlotIndex);
	
	UFUNCTION(BlueprintCallable)
	void EquipWeapon(int32 SlotIndex);
	
	virtual void GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const override;
	virtual bool ReplicateSubobjects(class UActorChannel* Channel, class FOutBunch* Bunch, FReplicationFlags* RepFlags) override;
	FORCEINLINE UPTWItemInstance* GetCurrentWeaponInst() const {return CurrentWeapon;}
	void SetCurrentWeaponInst(const UPTWItemInstance* WeaponInst);
	
	void WeaponVisibleSetting(const FGameplayTag& WeaponTag, bool bSetHidden);
	
	void ClearAndDestroyInventory();
	
	void SendEquipEventToASC(int32 SlotIndex, UAbilitySystemComponent* ASC);
	
	void SetWeaponActorHidden(UPTWItemInstance* Weapon, bool bInHidden);
protected:
	// Called when the game starts
	virtual void BeginPlay() override;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly,Replicated)
	TArray<TObjectPtr<UPTWItemInstance>> ItemArr; // 아이템 전체를 저장하는 배열
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Replicated)
	TObjectPtr<UPTWItemInstance> CurrentWeapon;
};
