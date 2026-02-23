// Fill out your copyright notice in the Description page of Project Settings.

#include "PTWGA_TimeKeeper.h"
#include "AbilitySystemComponent.h"
#include "GameplayEffect.h"
#include "CoreFramework/PTWPlayerCharacter.h"
#include "Inventory/PTWInventoryComponent.h"
#include "PTW/MiniGame/Item/BombItem/PTWBombActor.h"

UPTWGA_TimeKeeper::UPTWGA_TimeKeeper()
{
	NetExecutionPolicy = EGameplayAbilityNetExecutionPolicy::ServerOnly;
	InstancingPolicy   = EGameplayAbilityInstancingPolicy::InstancedPerActor;
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
		if (APTWBombActor* Found = Cast<APTWBombActor>(A))
		{
			BombActor = Found;
			break;
		}
	}

	if (!BombActor)
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, false);
		return;
	}
	
	bool bApplied = false;

	if (UAbilitySystemComponent* BombASC = BombActor->GetAbilitySystemComponent())
	{
		FGameplayEffectContextHandle Context = BombASC->MakeEffectContext();
		Context.AddSourceObject(this);

		FGameplayEffectSpecHandle SpecHandle = BombASC->MakeOutgoingSpec(AddTimeEffect, 1.0f, Context);
		if (SpecHandle.IsValid())
		{
			const FActiveGameplayEffectHandle AppliedHandle =
				BombASC->ApplyGameplayEffectSpecToSelf(*SpecHandle.Data.Get());

			bApplied = AppliedHandle.WasSuccessfullyApplied();
		}
	}

	if (bApplied)
	{
		ConsumeActiveItemIfPossible(ActorInfo);
	}

	EndAbility(Handle, ActorInfo, ActivationInfo, true, false);
}

void UPTWGA_TimeKeeper::ConsumeActiveItemIfPossible(const FGameplayAbilityActorInfo* ActorInfo) const
{
	APTWPlayerCharacter* PC = ActorInfo ? Cast<APTWPlayerCharacter>(ActorInfo->AvatarActor.Get()) : nullptr;
	if (!PC) return;

	UPTWInventoryComponent* Inven = PC->GetInventoryComponent();
	if (!Inven) return;
	
	Inven->ConsumeActiveItem();
}
