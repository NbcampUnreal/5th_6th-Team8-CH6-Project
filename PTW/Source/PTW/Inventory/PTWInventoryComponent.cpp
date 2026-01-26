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



void UPTWInventoryComponent::AddItem(TObjectPtr<UPTWItemDefinition> ItemClass, APTWWeaponActor* WeaponActor)
{
	//WeaponArr.AddUnique(AddItemDef)
	UPTWItemInstance* WeaponItemInst = NewObject<UPTWItemInstance>(this);
	WeaponItemInst->ItemDef = ItemClass;
	WeaponItemInst->SpawnedWeapon = WeaponActor;
	WeaponItemInst->CurrentAmmo = WeaponActor->GetWeaponData()->MaxAmmo;
	WeaponArr.Add(WeaponItemInst);
}

void UPTWInventoryComponent::SwapWeapon(int32 SlotIndex)
{
	
}

void UPTWInventoryComponent::EqiupWeapon(int32 SlotIndex)
{
	// TODO: 장착 GA 실행
	APawn* Pawn = Cast<APawn>(GetOwner());
	if (!Pawn) return;
	
	UAbilitySystemComponent* ASC = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(Pawn);
	if (!ASC) return;
	
	if (!WeaponArr.IsValidIndex(SlotIndex) || !WeaponArr[SlotIndex]) return;
	UPTWItemInstance* TargetInstance = WeaponArr[SlotIndex];
	CurrentWeapon = TargetInstance;
	
	bool bHasEquip = ASC->HasMatchingGameplayTag(FGameplayTag::RequestGameplayTag(FName("Weapon.State.Equip")));
	
	FGameplayTag EquipTag = bHasEquip ? FGameplayTag::RequestGameplayTag(FName("Weapon.State.UnEquip")) : 
	FGameplayTag::RequestGameplayTag(FName("Weapon.State.Equip"));
	
	
	FGameplayEventData Payload;
	Payload.OptionalObject = TargetInstance;
	
	ASC->HandleGameplayEvent(EquipTag, &Payload);
	
	ServerRPCSetCurrentWeapon(SlotIndex);
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


// Called when the game starts
void UPTWInventoryComponent::BeginPlay()
{
	Super::BeginPlay();
}

void UPTWInventoryComponent::ServerRPCSetCurrentWeapon_Implementation(int32 SlotIndex)
{
	CurrentWeapon = WeaponArr[SlotIndex];
}

bool UPTWInventoryComponent::ServerRPCSetCurrentWeapon_Validate(int32 SlotIndex)
{
	if (WeaponArr.IsValidIndex(SlotIndex))
	{
		return true;
	}
	return false;
}


void UPTWInventoryComponent::Server_EquipWeapon_Implementation(int32 SlotIndex)
{
	APawn* Pawn = Cast<APawn>(GetOwner());
	if (!Pawn) return;
	
	UAbilitySystemComponent* ASC = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(Pawn);
	if (!ASC) return;
	
	if (!WeaponArr.IsValidIndex(SlotIndex) || !WeaponArr[SlotIndex]) return;
	UPTWItemInstance* TargetInstance = WeaponArr[SlotIndex];
	CurrentWeapon = TargetInstance;
	
	//ServerRPCSetCurrentWeapon(SlotIndex);
	
	bool bHasEquip = ASC->HasMatchingGameplayTag(FGameplayTag::RequestGameplayTag(FName("Weapon.State.Equip")));
	
	FGameplayTag EquipTag = bHasEquip ? FGameplayTag::RequestGameplayTag(FName("Weapon.State.UnEquip")) : 
	FGameplayTag::RequestGameplayTag(FName("Weapon.State.Equip"));
	
	
	FGameplayEventData Payload;
	Payload.OptionalObject = TargetInstance;
	
	ASC->HandleGameplayEvent(EquipTag, &Payload);
}

bool UPTWInventoryComponent::Server_EquipWeapon_Validate(int32 SlotIndex)
{
	if (WeaponArr.IsValidIndex(SlotIndex))
	{
		return true;
	}
	
	return false;
}


