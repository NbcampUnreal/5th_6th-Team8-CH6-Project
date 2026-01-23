// Fill out your copyright notice in the Description page of Project Settings.


#include "GAS/Abilities/PTWGA_GimmickCollect.h"
#include "GameFramework/Actor.h"
#include "GameplayTagsManager.h"

UPTWGA_GimmickCollect::UPTWGA_GimmickCollect()
{
	// 서버에서만 실행
	NetExecutionPolicy = EGameplayAbilityNetExecutionPolicy::ServerOnly;
	InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;

	// GameplayEvent로 발동되게 설정
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

	// 기믹 액터는 OptionalObject로 전달됨
	AActor* GimmickActor =
		TriggerEventData ? Cast<AActor>(TriggerEventData->OptionalObject) : nullptr;

	if (IsValid(GimmickActor))
	{
		GimmickActor->Destroy();
	}

	EndAbility(Handle, ActorInfo, ActivationInfo, true, false);
}
