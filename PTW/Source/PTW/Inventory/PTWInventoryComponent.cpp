// Fill out your copyright notice in the Description page of Project Settings.

#include "PTWInventoryComponent.h"


UPTWInventoryComponent::UPTWInventoryComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
	
	//SetIsReplicated(true);
}

void UPTWInventoryComponent::AddItem(const UPTWItemDefinition& AddItemDef)
{
	
}

void UPTWInventoryComponent::SwapWeapon(int32 SlotIndex)
{
	
}


// Called when the game starts
void UPTWInventoryComponent::BeginPlay()
{
	Super::BeginPlay();
}


