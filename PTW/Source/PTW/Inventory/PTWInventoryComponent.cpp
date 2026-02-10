// Fill out your copyright notice in the Description page of Project Settings.

#include "PTWInventoryComponent.h"

#include "AbilitySystemBlueprintLibrary.h"
#include "AbilitySystemComponent.h"
#include "PTWItemDefinition.h"
#include "Instance/PTWItemInstance.h"
#include "../Weapon/PTWWeaponActor.h"
#include "CoreFramework/PTWCombatInterface.h"
#include "Engine/ActorChannel.h"
#include "GAS/PTWGameplayAbility.h"
#include "GAS/Abilities/PTWGA_Fire.h"
#include "Instance/PTWActiveItemInstance.h"
#include "Instance/PTWPassiveItemInstance.h"
#include "Instance/PTWWeaponInstance.h"
#include "Net/UnrealNetwork.h"
#include "PTWGameplayTag/GameplayTags.h"


UPTWInventoryComponent::UPTWInventoryComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
	SetIsReplicatedByDefault(true);
}



void UPTWInventoryComponent::AddItem(TObjectPtr<UPTWItemInstance> ItemClass)
{
	if (!GetOwner()->HasAuthority()) return;
	ItemArr.Add(ItemClass);
	
	OnItemInstanceCreated(ItemClass);
}

void UPTWInventoryComponent::SwapWeapon(int32 SlotIndex)
{
	
}

void UPTWInventoryComponent::EquipWeapon(int32 SlotIndex)
{
	// TODO: 장착 GA 실행
	APawn* Pawn = Cast<APawn>(GetOwner());
	if (!Pawn) return;
	
	UAbilitySystemComponent* ASC = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(Pawn);
	if (!ASC) return;
	
	SendEquipEventToASC(SlotIndex, ASC);
}

void UPTWInventoryComponent::GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	
	DOREPLIFETIME(UPTWInventoryComponent, ItemArr);
	DOREPLIFETIME(UPTWInventoryComponent, CurrentWeapon);
	DOREPLIFETIME(UPTWInventoryComponent, CurrentActiveItemSlot);
	DOREPLIFETIME(UPTWInventoryComponent, CurSelectingWeaponSlot);
}

 bool UPTWInventoryComponent::ReplicateSubobjects(class UActorChannel* Channel, class FOutBunch* Bunch,
 	FReplicationFlags* RepFlags)
 {
 	bool WroteSomething = Super::ReplicateSubobjects(Channel, Bunch, RepFlags);
 	
 	for (UPTWItemInstance* Item : ItemArr)
 	{
 		if (Item)
 		{
 			WroteSomething |= Channel->ReplicateSubobject(Item, *Bunch, *RepFlags);
 		}
 	}
	WroteSomething |= Channel->ReplicateSubobject(CurrentActiveItemSlot, *Bunch, *RepFlags);
 	return WroteSomething;
 }

void UPTWInventoryComponent::SetCurrentWeaponInst(const UPTWItemInstance* WeaponInst)
{
	CurrentWeapon = const_cast<UPTWItemInstance*>(WeaponInst);
}

void UPTWInventoryComponent::WeaponVisibleSetting(const FGameplayTag& WeaponTag, bool bSetHidden)
{
	for (auto Weapon : ItemArr)
	{
		if (Weapon && Weapon->ItemDef && Weapon->ItemDef->WeaponTag == WeaponTag)
		{
			SetWeaponActorHidden(Weapon, bSetHidden);
		}
	}
}

void UPTWInventoryComponent::OnItemInstanceCreated(UPTWItemInstance* ItemInstance)
{
	if (!ItemInstance || !GetOwner()->HasAuthority()) return;
	
	if (ItemInstance->IsA(UPTWPassiveItemInstance::StaticClass()))
	{
		ApplyAllPassiveItems(ItemInstance);
	}
	
	if (ItemInstance->IsA(UPTWActiveItemInstance::StaticClass()))
	{
		EquipActiveItem(ItemInstance);
	}
	
	if (ItemInstance->IsA(UPTWWeaponInstance::StaticClass()))
	{
		WeaponArr.Add(Cast<UPTWWeaponInstance>(ItemInstance));
	}
}


void UPTWInventoryComponent::ApplyAllPassiveItems(UPTWItemInstance* ItemInstance)
{
	UAbilitySystemComponent* ASC = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(GetOwner());
		
	if (!ASC) return;
		
	UPTWPassiveItemInstance* PassiveItemInstance = Cast<UPTWPassiveItemInstance>(ItemInstance);
	UPTWItemDefinition* ItemDef = PassiveItemInstance->ItemDef;
		
	if (PassiveItemInstance && ItemDef &&ItemDef->PassiveEffects.Num() > 0)
	{
		for (auto GEClass : ItemDef->PassiveEffects)
		{
			FGameplayEffectContextHandle Context = ASC->MakeEffectContext();
			FGameplayEffectSpecHandle SpecHandle = ASC->MakeOutgoingSpec(GEClass, 1.0f, Context);
            
			if (SpecHandle.IsValid())
			{
				FActiveGameplayEffectHandle ActiveHandle = ASC->ApplyGameplayEffectSpecToSelf(*SpecHandle.Data.Get());
				PassiveItemInstance->AddPassiveSpecHandles(ActiveHandle);
			}
		}
	}
}


void UPTWInventoryComponent::RemoveAllPassiveItems(UPTWItemInstance* ItemInstance)
{
	if (!GetOwner()->HasAuthority()) return;
	
	UAbilitySystemComponent* ASC = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(GetOwner());
	if (ASC)
	{
		UPTWPassiveItemInstance* PassiveItemInstance = Cast<UPTWPassiveItemInstance>(ItemInstance);
		for (const FActiveGameplayEffectHandle& Handle : PassiveItemInstance->GetPassiveSpecHandleArr())
		{
			if (Handle.IsValid())
			{
				PassiveItemInstance->RemovePassiveSpecHandles(Handle);
				ASC->RemoveActiveGameplayEffect(Handle);
			}
		}
	}
}

void UPTWInventoryComponent::RemoveWeaponItem()
{
 	if (WeaponArr[CurSelectingWeaponSlot])
	{
 		int32 tempIndex = CurSelectingWeaponSlot;
		WeaponArr[CurSelectingWeaponSlot]->DestroySpawnedActors();
		SendEquipEventToASC(CurSelectingWeaponSlot, UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(GetOwner()));
		WeaponArr.RemoveAt(tempIndex);
	}
}

// 해당 무기 전용 어빌리티 및 태그 적용
void UPTWInventoryComponent::ApplyWeaponData()
{
	UPTWItemInstance* Inst = CurrentWeapon;
	
	if (Inst || Inst->ItemDef->AbilityToGrant)
	{
		UAbilitySystemComponent* ASC = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(GetOwner());
		if (!ASC) return;
		
		FGameplayAbilitySpec Spec = FGameplayAbilitySpec(Inst->ItemDef->AbilityToGrant, 1, INDEX_NONE, GetOwner());
		
		if (const UPTWGameplayAbility* WeaponAbility = Cast<UPTWGameplayAbility>(Inst->ItemDef->AbilityToGrant->GetDefaultObject()))
		{
			if (WeaponAbility->StartupInputTag.IsValid())
			{
				Spec.DynamicAbilityTags.AddTag(WeaponAbility->StartupInputTag);
			}
		}
		
		FGameplayAbilitySpecHandle SpecHandle = ASC->GiveAbility(Spec);
		CurrentWeaponAbilitySpec = SpecHandle;
	}
	
	if (Inst->ItemDef->EffectToGrant)
	{
		IPTWCombatInterface* CombatInt = Cast<IPTWCombatInterface>(GetOwner());
		if (CombatInt)
		{
			CombatInt->ApplyGameplayEffectToSelf(Inst->ItemDef->EffectToGrant, 1.0f, FGameplayEffectContextHandle());
		}
	}
}

// 제거
void UPTWInventoryComponent::RemoveWeaponData()
{
	if (UAbilitySystemComponent* ASC = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(GetOwner()))
	{
		FGameplayTag Tags = ASC->HasMatchingGameplayTag(GameplayTags::Weapon::EquipType::Basic) ?GameplayTags::Weapon::EquipType::Basic
		: GameplayTags::Weapon::EquipType::Special;
		
		if (IPTWCombatInterface* CombatInt = Cast<IPTWCombatInterface>(GetOwner()))
		{
			CombatInt->RemoveEffectWithTag(Tags);
		}
		
		ASC->ClearAbility(CurrentWeaponAbilitySpec);
		CurrentWeaponAbilitySpec = FGameplayAbilitySpecHandle();
	}
}


// Called when the game starts
void UPTWInventoryComponent::BeginPlay()
{
	Super::BeginPlay();
}

void UPTWInventoryComponent::SendGameplayEvent(UPTWItemInstance* ItemInstance, FGameplayTag SendTag, int32 SlotIndex)
{
	if (!GetOwner()->HasAuthority()) return;
	
	FGameplayEventData Payload;
	Payload.OptionalObject = ItemInstance;
	Payload.EventMagnitude = static_cast<float>(SlotIndex);
	Payload.Instigator = GetOwner();
	
	UAbilitySystemBlueprintLibrary::SendGameplayEventToActor(GetOwner(), SendTag, Payload);
}


void UPTWInventoryComponent::ClearAndDestroyInventory()
{
	CurrentWeapon = nullptr;

	for (UPTWItemInstance* Item : ItemArr)
	{
		if (!Item) continue;
		if (UPTWWeaponInstance* WeaponItemInst = Cast<UPTWWeaponInstance>(Item))
		{
			WeaponItemInst->DestroySpawnedActors(); 
		}
		
	}

	ItemArr.Empty();
}

void UPTWInventoryComponent::SendEquipEventToASC(int32 SlotIndex, UAbilitySystemComponent* ASC)
{
	if (!WeaponArr.IsValidIndex(SlotIndex) || !WeaponArr[SlotIndex]) return;
	if (!GetOwner()->HasAuthority()) return;
	
	UPTWItemInstance* TargetInstance = nullptr;
	FGameplayTag SendTag;
	
	if (CurSelectingWeaponSlot != SlotIndex) // 이전에 선택한 슬롯 인덱스와 다른 슬롯 인덱스인 경우 장착 해제 후 장착
	{
		if (WeaponArr.IsValidIndex(CurSelectingWeaponSlot))
		{
			RemoveWeaponData();
			SendTag = GameplayTags::Weapon::State::UnEquip;
			SendGameplayEvent(CurrentWeapon, SendTag, SlotIndex);
		}
		
		TargetInstance = WeaponArr[SlotIndex];
		CurrentWeapon = TargetInstance;
		SendTag = GameplayTags::Weapon::State::Equip;
		CurSelectingWeaponSlot = SlotIndex;
		ApplyWeaponData();
	}
	else // 같은 슬롯 인덱스인 경우 장착 해제
	{
		RemoveWeaponData();
		TargetInstance = CurrentWeapon;
		SendTag = GameplayTags::Weapon::State::UnEquip;
		CurrentWeapon = nullptr;
		CurSelectingWeaponSlot = -1;
	}
	
	SendGameplayEvent(TargetInstance, SendTag, SlotIndex);
}

void UPTWInventoryComponent::SetWeaponActorHidden(UPTWItemInstance* Weapon, bool bInHidden)
{
	if (!Weapon) return;
	if (UPTWWeaponInstance* WeaponInstance = Cast<UPTWWeaponInstance>(Weapon))
	{
		if (WeaponInstance->SpawnedWeapon1P) WeaponInstance->SpawnedWeapon1P->SetActorHiddenInGame(bInHidden);
		if (WeaponInstance->SpawnedWeapon3P) WeaponInstance->SpawnedWeapon3P->SetActorHiddenInGame(bInHidden);
	}
}

void UPTWInventoryComponent::UseActiveItem()
{
	UAbilitySystemComponent* ASC = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(GetOwner());
	
	if (ASC && ActiveItemAbilityHandle.IsValid())
	{
		ASC->TryActivateAbility(ActiveItemAbilityHandle);
	}
}


bool UPTWInventoryComponent::EquipActiveItem(UPTWItemInstance* ActiveItemInstance)
{
	if (CurrentActiveItemSlot) return false; // 이미 장착된 아이템이 있다면
	if (!ActiveItemInstance || !ActiveItemInstance->ItemDef->AbilityToGrant) return false; 
	
	CurrentActiveItemSlot = ActiveItemInstance;

	UAbilitySystemComponent* ASC = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(GetOwner());
	if (ASC)
	{
		if (ActiveItemAbilityHandle.IsValid())
		{
			ASC->ClearAbility(ActiveItemAbilityHandle);
		}
		ActiveItemAbilityHandle = ASC->GiveAbility(FGameplayAbilitySpec(CurrentActiveItemSlot->ItemDef->AbilityToGrant, 1));
	}
	
	return true;
}

void UPTWInventoryComponent::ConsumeActiveItem()
{
	if (ActiveItemAbilityHandle.IsValid())
	{
		if (UAbilitySystemComponent* ASC = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(GetOwner()))
		{
			ASC->ClearAbility(ActiveItemAbilityHandle);
			ActiveItemAbilityHandle = FGameplayAbilitySpecHandle();
		}
	}
	
	CurrentActiveItemSlot = nullptr;
}

