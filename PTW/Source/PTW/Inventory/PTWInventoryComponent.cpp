// Fill out your copyright notice in the Description page of Project Settings.

#include "PTWInventoryComponent.h"

#include "PTWItemDefinition.h"
#include "PTWItemInstance.h"
#include "PTWWeaponActor.h"
#include "CoreFramework/PTWBaseCharacter.h"


UPTWInventoryComponent::UPTWInventoryComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
	SetIsReplicatedByDefault(true);
}

void UPTWInventoryComponent::AddItem(TSubclassOf<UPTWItemDefinition> ItemClass)
{
	//WeaponArr.AddUnique(AddItemDef)
	UPTWItemInstance* WeaponItemInst = NewObject<UPTWItemInstance>(this);
	WeaponItemInst->ItemDef = ItemClass->GetDefaultObject<UPTWItemDefinition>();
	WeaponArr.Add(WeaponItemInst);
}

void UPTWInventoryComponent::SwapWeapon(int32 SlotIndex)
{
	
}

void UPTWInventoryComponent::EqiupWeapon(int32 SlotIndex)
{
	UPTWItemInstance* TargetInstance = WeaponArr[SlotIndex];
	if (!TargetInstance) return;
	
	// TODO: 장착 GA 실행
}


// Called when the game starts
void UPTWInventoryComponent::BeginPlay()
{
	Super::BeginPlay();
}


