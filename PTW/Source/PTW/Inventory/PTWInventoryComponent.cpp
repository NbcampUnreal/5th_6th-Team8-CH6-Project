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
	//WeaponArr.AddUnique(AddItemDef)
	WeaponArr.Add(ItemClass);
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
	Payload.EventMagnitude = static_cast<float>(SlotIndex);
	
	ASC->HandleGameplayEvent(EquipTag, &Payload);
}

void UPTWInventoryComponent::GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(UPTWInventoryComponent, WeaponArr);
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

// Called when the game starts
void UPTWInventoryComponent::BeginPlay()
{
	Super::BeginPlay();
}


