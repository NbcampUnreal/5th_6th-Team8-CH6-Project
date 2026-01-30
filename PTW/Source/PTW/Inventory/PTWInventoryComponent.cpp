// Fill out your copyright notice in the Description page of Project Settings.

#include "PTWInventoryComponent.h"

#include "AbilitySystemBlueprintLibrary.h"
#include "AbilitySystemComponent.h"
#include "PTWItemDefinition.h"
#include "PTWItemInstance.h"
#include "PTWWeaponActor.h"
#include "PTWWeaponData.h"
#include "CoreFramework/PTWBaseCharacter.h"
#include "Engine/ActorChannel.h"
#include "Net/UnrealNetwork.h"


UPTWInventoryComponent::UPTWInventoryComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
	SetIsReplicatedByDefault(true);
}



void UPTWInventoryComponent::AddItem(TObjectPtr<UPTWItemInstance> ItemClass)
{
	WeaponArr.Add(ItemClass);
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
	
	DOREPLIFETIME(UPTWInventoryComponent, WeaponArr);
	DOREPLIFETIME(UPTWInventoryComponent, CurrentWeapon);
}

 bool UPTWInventoryComponent::ReplicateSubobjects(class UActorChannel* Channel, class FOutBunch* Bunch,
 	FReplicationFlags* RepFlags)
 {
 	bool WroteSomething = Super::ReplicateSubobjects(Channel, Bunch, RepFlags);
 	
 	for (UPTWItemInstance* Item : WeaponArr)
 	{
 		if (Item)
 		{
 			WroteSomething |= Channel->ReplicateSubobject(Item, *Bunch, *RepFlags);
 		}
 	}
	
 	return WroteSomething;
 }

void UPTWInventoryComponent::SetCurrentWeaponInst(const UPTWItemInstance* WeaponInst)
{
	CurrentWeapon = const_cast<UPTWItemInstance*>(WeaponInst);
}

void UPTWInventoryComponent::WeaponVisibleSetting(const FGameplayTag& WeaponTag, bool bSetHidden)
{
	for (auto Weapon : WeaponArr)
	{
		if (Weapon && Weapon->ItemDef && Weapon->ItemDef->WeaponTag == WeaponTag)
		{
			SetWeaponActorHidden(Weapon, bSetHidden);
		}
	}
}

// Called when the game starts
void UPTWInventoryComponent::BeginPlay()
{
	Super::BeginPlay();
}


void UPTWInventoryComponent::ClearAndDestroyInventory()
{
	CurrentWeapon = nullptr;

	for (UPTWItemInstance* Item : WeaponArr)
	{
		if (!Item) continue;
		Item->DestroySpawnedActors(); 
	}

	WeaponArr.Empty();
}

void UPTWInventoryComponent::SendEquipEventToASC(int32 SlotIndex, UAbilitySystemComponent* ASC)
{
	if (!WeaponArr.IsValidIndex(SlotIndex) || !WeaponArr[SlotIndex]) return;
	UPTWItemInstance* TargetInstance = WeaponArr[SlotIndex];
	CurrentWeapon = TargetInstance;
	
	bool bHasEquip = ASC->HasMatchingGameplayTag(FGameplayTag::RequestGameplayTag(FName("Weapon.State.Equip")));
	
	FGameplayTag EquipTag = bHasEquip ? FGameplayTag::RequestGameplayTag(FName("Weapon.State.UnEquip")) : 
	FGameplayTag::RequestGameplayTag(FName("Weapon.State.Equip"));
	
	FGameplayEventData Payload;
	Payload.OptionalObject = TargetInstance;
	Payload.EventMagnitude = static_cast<float>(SlotIndex);
	Payload.Instigator = GetOwner();
	
	ASC->HandleGameplayEvent(EquipTag, &Payload);
}

void UPTWInventoryComponent::SetWeaponActorHidden(UPTWItemInstance* Weapon, bool bInHidden)
{
	if (!Weapon) return;
	if (Weapon->SpawnedWeapon1P) Weapon->SpawnedWeapon1P->SetActorHiddenInGame(bInHidden);
	if (Weapon->SpawnedWeapon3P) Weapon->SpawnedWeapon3P->SetActorHiddenInGame(bInHidden);
}
