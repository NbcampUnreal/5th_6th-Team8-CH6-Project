// Fill out your copyright notice in the Description page of Project Settings.


#include "GAS/Abilities/PTWGA_Interact.h"
#include "CoreFramework/Character/Component/PTWInteractComponent.h"
#include "CoreFramework/PTWPlayerCharacter.h"

UPTWGA_Interact::UPTWGA_Interact()
{

}

bool UPTWGA_Interact::CanActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayTagContainer* SourceTags, const FGameplayTagContainer* TargetTags, OUT FGameplayTagContainer* OptionalRelevantTags) const
{
	if (!Super::CanActivateAbility(Handle, ActorInfo, SourceTags, TargetTags, OptionalRelevantTags))
	{
		return false;
	}
	if (APTWPlayerCharacter* Character = Cast<APTWPlayerCharacter>(ActorInfo->AvatarActor.Get()))
	{
		if (UPTWInteractComponent* InteractComp = Character->GetInteractComponent())
		{
			return InteractComp->HasValidTarget();
		}
	}
	return false;
}

void UPTWGA_Interact::ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData)
{
	if (!CommitAbility(Handle, ActorInfo, ActivationInfo))
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, false);
		return;
	}
	if (APTWPlayerCharacter* Character = Cast<APTWPlayerCharacter>(ActorInfo->AvatarActor.Get()))
	{
		if (UPTWInteractComponent* InteractComp = Character->GetInteractComponent())
		{
			InteractComp->PerformInteraction();
		}
	}
	EndAbility(Handle, ActorInfo, ActivationInfo, true, false);
}
