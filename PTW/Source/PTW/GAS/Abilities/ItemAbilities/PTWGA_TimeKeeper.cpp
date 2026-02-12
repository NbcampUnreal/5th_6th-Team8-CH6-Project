// Fill out your copyright notice in the Description page of Project Settings.

#include "PTWGA_TimeKeeper.h"

#include "GameplayTagContainer.h"
#include "PTW/MiniGame/Item/BombItem/PTWBombActor.h"
#include "AbilitySystemComponent.h"

UPTWGA_TimeKeeper::UPTWGA_TimeKeeper()
{
	NetExecutionPolicy = EGameplayAbilityNetExecutionPolicy::ServerOnly;
	InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;
}

void UPTWGA_TimeKeeper::ActivateAbility(
	const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo,
	const FGameplayAbilityActivationInfo ActivationInfo,
	const FGameplayEventData* TriggerEventData)
{
	Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);
	
	if (!HasAuthority(&ActivationInfo) || !AddTimeEffect)
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, false);
		return;
	}

	APawn* AvatarPawn = ActorInfo ? Cast<APawn>(ActorInfo->AvatarActor.Get()) : nullptr;
	if (!AvatarPawn)
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, false);
		return;
	}
	
	TArray<AActor*> AttachedActors;
	AvatarPawn->GetAttachedActors(AttachedActors);

	APTWBombActor* BombActor = nullptr;
	for (AActor* A : AttachedActors)
	{
		BombActor = Cast<APTWBombActor>(A);
		if (BombActor) break;
	}

	if (!BombActor)
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, false);
		return;
	}
	
	UAbilitySystemComponent* BombASC = BombActor->GetAbilitySystemComponent();
	if (BombASC)
	{
		FGameplayEffectContextHandle Context = BombASC->MakeEffectContext();
		Context.AddSourceObject(this);

		FGameplayEffectSpecHandle SpecHandle =
			BombASC->MakeOutgoingSpec(AddTimeEffect, 1.0f, Context);

		if (SpecHandle.IsValid())
		{
			BombASC->ApplyGameplayEffectSpecToSelf(*SpecHandle.Data.Get());
		}
	}

	EndAbility(Handle, ActorInfo, ActivationInfo, true, false);
}
