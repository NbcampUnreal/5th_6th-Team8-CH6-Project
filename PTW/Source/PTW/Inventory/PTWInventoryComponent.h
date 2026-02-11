// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameplayAbilitySpecHandle.h"
#include "Components/ActorComponent.h"
#include "PTWInventoryComponent.generated.h"


class UPTWActiveItemInstance;
class UGameplayAbility;
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
	
	/*사용 아이템 사용 함수*/
	UFUNCTION(BlueprintCallable)
	void UseActiveItem();
	
	/*필드에 드랍된 아이템을 먹거나, 상점에서 구입한 사용 아이템은 해당 함수 호출 */
	UFUNCTION(BlueprintCallable)
	bool EquipActiveItem(UPTWItemInstance* ActiveItemInstance);
	
	void ConsumeActiveItem();
	
	void OnItemInstanceCreated(UPTWItemInstance* ItemInstance);
	
	void ApplyAllPassiveItems(UPTWItemInstance* ItemInstance);
	
	void RemoveAllPassiveItems(UPTWItemInstance* ItemInstance);
	
	void RemoveWeaponItem();
	
	void ApplyWeaponData();
	
	void RemoveWeaponData();
	
	// FIXME : 테스트 코드
	UFUNCTION(BlueprintCallable, Server, Reliable)
	void TestFunction_GiveActiveItem(UPTWItemDefinition* Def);
	
protected:
	// Called when the game starts
	virtual void BeginPlay() override;
	
	void SendGameplayEvent(UPTWItemInstance* ItemInstance, FGameplayTag SendTag, int32 SlotIndex);
	
protected:
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly,Replicated)
	TArray<TObjectPtr<UPTWItemInstance>> ItemArr; // 아이템 전체를 저장하는 배열
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Replicated)
	TObjectPtr<UPTWItemInstance> CurrentWeapon;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Replicated)
	TObjectPtr<UPTWActiveItemInstance> CurrentActiveItemSlot;
	
	FGameplayAbilitySpecHandle ActiveItemAbilityHandle;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Replicated)
	int32 CurSelectingWeaponSlot = -1;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	TSubclassOf<UGameplayAbility> Test;
	
private:
	TArray<TObjectPtr<UPTWWeaponInstance>> WeaponArr;
	
	FGameplayAbilitySpecHandle CurrentWeaponAbilitySpec;
};
