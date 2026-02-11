// Fill out your copyright notice in the Description page of Project Settings.


#include "PTWActiveItemInstance.h"

#include "Net/UnrealNetwork.h"

void UPTWActiveItemInstance::GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	
	DOREPLIFETIME(ThisClass, CurrentCount);
}

bool UPTWActiveItemInstance::UsingActiveItem()
{
	if (CurrentCount <= 0) return false;
	CurrentCount--;
	
	return true;
}
