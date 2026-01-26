// Fill out your copyright notice in the Description page of Project Settings.

#include "GAS/Abilities/PTWGA_GimmickCollect.h"

#include "GameFramework/Actor.h"
#include "GameplayTagContainer.h"    
#include "GameplayTagsManager.h"  

UPTWGA_GimmickCollect::UPTWGA_GimmickCollect()
{
	NetExecutionPolicy = EGameplayAbilityNetExecutionPolicy::ServerOnly;
	InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;
	
	FAbilityTriggerData TriggerData;
	TriggerData.TriggerTag = FGameplayTag::RequestGameplayTag(TEXT("Event.Gimmick.Collect"));
	TriggerData.TriggerSource = EGameplayAbilityTriggerSource::GameplayEvent;

	AbilityTriggers.Add(TriggerData);
}

void UPTWGA_GimmickCollect::ActivateAbility(
	const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo,
	const FGameplayAbilityActivationInfo ActivationInfo,
	const FGameplayEventData* TriggerEventData)
{
	if (!ActorInfo || !ActorInfo->IsNetAuthority())
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, false);
		return;
	}


	const AActor* GimmickActorConst =
		TriggerEventData ? Cast<AActor>(TriggerEventData->OptionalObject) : nullptr;

	AActor* GimmickActor = const_cast<AActor*>(GimmickActorConst);

	if (IsValid(GimmickActor))
	{
		GimmickActor->Destroy();
	}

	EndAbility(Handle, ActorInfo, ActivationInfo, true, false);
}
