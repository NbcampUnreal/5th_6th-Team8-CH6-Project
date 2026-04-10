// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameplayAbilitySpecHandle.h"
#include "Components/ActorComponent.h"
#include "CoreFramework/Character/Component/PTWWeaponComponent.h"
#include "PTWInventoryComponent.generated.h"


class UPTWActiveItemInstance;
class UGameplayAbility;
struct FGameplayTag;
class UPTWItemInstance;
class APTWWeaponActor;
class UPTWItemDefinition;
class UAbilitySystemComponent;
class UPTWWeaponInstance;


DECLARE_MULTICAST_DELEGATE(FOnInventoryChanged);


UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class PTW_API UPTWInventoryComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	// Sets default values for this component's properties
	UPTWInventoryComponent();
	
	void AddItem(TObjectPtr<UPTWItemInstance>ItemClass);
	
	UFUNCTION(BlueprintCallable)
	void EquipWeapon(int32 SlotIndex);
	
	virtual void GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const override;
	
	virtual bool ReplicateSubobjects(class UActorChannel* Channel, class FOutBunch* Bunch, FReplicationFlags* RepFlags) override;
	
	template<typename T>
	FORCEINLINE T* GetCurrentWeaponInst() const { return Cast<T>(CurrentWeapon); }
	
	FORCEINLINE TArray<TObjectPtr<UPTWWeaponInstance>> GetWeaponArray() const { return WeaponArr;}
	
	/* WeaponInstance Setter*/
	void SetCurrentWeaponInst(const UPTWItemInstance* WeaponInst);
	
	/* 무기 비주얼 세팅 */
	void WeaponVisibleSetting(const FGameplayTag& WeaponTag, bool bSetHidden);
	
	/* 인벤토리 Clear */
	void ClearAndDestroyInventory();
	
	/* 장착 시 이벤트 보내주는 함수 */
	void SendEquipEventToASC(int32 SlotIndex);
	
	/* 무기 액터 가려주는 함수*/
	void SetWeaponActorHidden(UPTWItemInstance* Weapon, bool bInHidden);
	
	FORCEINLINE int32 GetCurrentSlotIndex() const { return CurSelectingWeaponSlot;}
	
	/*사용 아이템 사용 함수*/
	UFUNCTION(BlueprintCallable)
	void UseActiveItem();
	
	/*필드에 드랍된 아이템을 먹거나, 상점에서 구입한 사용 아이템은 해당 함수 호출 */
	UFUNCTION(BlueprintCallable)
	bool EquipActiveItem(UPTWItemInstance* ActiveItemInstance);
	
	/* 액티브 아이템 사용 함수 */
	void ConsumeActiveItem();
	
	/* 추가된 아이템 Instance에 대한 설정 */
	bool OnItemInstanceCreated(UPTWItemInstance* ItemInstance);
	
	/* Passive 아이템 GE 적용 */
	void ApplyAllPassiveItems(UPTWItemInstance* ItemInstance);
	
	/* Passive 아이템 GE 제거*/
	void RemoveAllPassiveItems(UPTWItemInstance* ItemInstance);
	
	/* 무기 아이템 제거 */
	void RemoveWeaponItem();
	
	/* 무기 아이템 GA 적용 */
	void ApplyWeaponData();
	
	/* 무기 아이템 GA 제거 */
	void RemoveWeaponData();
	
	/* 무기 드랍 */
	void DropItem();
	
	FORCEINLINE const TArray<TObjectPtr<UPTWItemInstance>>& GetAllItems() const { return ItemArr; }
	
	/* 액티브 아이템 사용 시 부여된 SpecHandle 제거 */
	void RemoveActiveItemGameplayAbilityHandle();
	
	void AddWeaponArr(UPTWWeaponInstance* WeaponInst);
	
	/* 무기 배열 전부 제거 */
	void ClearWeaponArr();
	
protected:
	// Called when the game starts
	virtual void BeginPlay() override;
	
	void SendGameplayEvent(UPTWItemInstance* ItemInstance, FGameplayTag SendTag, int32 SlotIndex);
	
public:
	/* 델리게이트 */
	FOnInventoryChanged OnInventoryChanged;
	
protected:
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly,Replicated)
	TArray<TObjectPtr<UPTWItemInstance>> ItemArr; // 아이템 전체를 저장하는 배열
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Replicated)
	TObjectPtr<UPTWItemInstance> CurrentWeapon;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Replicated)
	TObjectPtr<UPTWActiveItemInstance> CurrentActiveItemSlot;
	
	UPROPERTY(Replicated)
	FGameplayAbilitySpecHandle ActiveItemAbilityHandle;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Replicated)
	int32 CurSelectingWeaponSlot = -1;

private:
	TArray<TObjectPtr<UPTWWeaponInstance>> WeaponArr;
	
	FGameplayAbilitySpecHandle CurrentWeaponAbilitySpec;
	
	TMap<AController*, FSavedWeaponData> SavedWeaponMaps;
	
	const int32 MAX_SLOT_COUNT = 3;
};
