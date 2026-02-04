// Fill out your copyright notice in the Description page of Project Settings.


#include "PTWPassiveItemInstance.h"

void UPTWPassiveItemInstance::AddPassiveSpecHandles(const FActiveGameplayEffectHandle& SpecHandle)
{
	PassiveSpecHandles.Add(SpecHandle);
}

void UPTWPassiveItemInstance::RemovePassiveSpecHandles(const FActiveGameplayEffectHandle& SpecHandle)
{
	PassiveSpecHandles.Remove(SpecHandle);
}

void UPTWPassiveItemInstance::RemoveAllPassiveSpecHandles()
{
	PassiveSpecHandles.Empty();
}
