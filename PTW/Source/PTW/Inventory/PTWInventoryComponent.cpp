// Fill out your copyright notice in the Description page of Project Settings.

#include "PTWInventoryComponent.h"

#include "AbilitySystemBlueprintLibrary.h"
#include "AbilitySystemComponent.h"
#include "PTWItemDefinition.h"
#include "PTWItemInstance.h"
#include "PTWWeaponActor.h"
#include "PTWWeaponData.h"
#include "CoreFramework/PTWBaseCharacter.h"
#include "GeometryCollection/GeometryCollectionParticlesData.h"


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
}


// Called when the game starts
void UPTWInventoryComponent::BeginPlay()
{
	Super::BeginPlay();
}


