// Fill out your copyright notice in the Description page of Project Settings.

#include "GAS/Abilities/PTWGA_GimmickCollect.h"

#include "Engine/World.h"
#include "GameFramework/Actor.h"
#include "GameFramework/Pawn.h"
#include "GameplayTagContainer.h"
#include "PTW/MiniGame/PTWMiniGameMode.h"

#include "CoreFramework/PTWBaseCharacter.h"

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

	APTWBaseCharacter* Character = GetPTWCharacterFromActorInfo();
	if (!Character && ActorInfo->AvatarActor.IsValid())
	{
		Character = Cast<APTWBaseCharacter>(ActorInfo->AvatarActor.Get());
	}

	if (Character)
	{
		if (UWorld* World = Character->GetWorld())
		{
			if (APTWMiniGameMode* MiniGameMode = World->GetAuthGameMode<APTWMiniGameMode>())
			{
				MiniGameMode->AddWinPoint(Cast<APawn>(Character), 1);
			}
		}
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
