// Fill out your copyright notice in the Description page of Project Settings.


#include "PTWGA_Banana.h"

void UPTWGA_Banana::ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo,
	const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData)
{
	Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);
}

void UPTWGA_Banana::InitializeVariable()
{
	Super::InitializeVariable();
}

void UPTWGA_Banana::ApplyItemEffect()
{
	UE_LOG(LogTemp, Warning, TEXT("Banana 사용"));
}
