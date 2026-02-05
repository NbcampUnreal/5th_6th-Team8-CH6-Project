// Fill out your copyright notice in the Description page of Project Settings.


#include "GAS/Abilities/PTWGA_Interact.h"
#include "CoreFramework/Character/Component/PTWInteractComponent.h"
#include "CoreFramework/PTWPlayerCharacter.h"
#include "CoreFramework/Interface/PTWInteractInterface.h"

UPTWGA_Interact::UPTWGA_Interact()
{

}

bool UPTWGA_Interact::CanActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayTagContainer* SourceTags, const FGameplayTagContainer* TargetTags, OUT FGameplayTagContainer* OptionalRelevantTags) const
{
	if (!Super::CanActivateAbility(Handle, ActorInfo, SourceTags, TargetTags, OptionalRelevantTags))
	{
		return false;
	}
	if (ActorInfo->AvatarActor->HasAuthority())
	{
		return true;
	}
	else
	{
		if (APTWPlayerCharacter* Character = Cast<APTWPlayerCharacter>(ActorInfo->AvatarActor.Get()))
		{
			if (UPTWInteractComponent* InteractComp = Character->GetInteractComponent())
			{
				return InteractComp->HasValidTarget();
			}
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
	if (ActorInfo->AvatarActor->HasAuthority())
	{
		if (APTWPlayerCharacter* Character = Cast<APTWPlayerCharacter>(ActorInfo->AvatarActor.Get()))
		{
			if (UPTWInteractComponent* InteractComp = Character->GetInteractComponent())
			{
				AActor* TargetActor = InteractComp->GetInteractTargetUnsafe();

				if (TargetActor)
				{
					if (TargetActor->Implements<UPTWInteractInterface>())
					{
						IPTWInteractInterface::Execute_OnInteract(TargetActor, Character);
						UE_LOG(LogTemp, Log, TEXT("[GA_Interact] Interaction Success: %s"), *TargetActor->GetName());
					}
				}
				else
				{
					UE_LOG(LogTemp, Warning, TEXT("[GA_Interact] Server Trace Failed - No Target"));
				}
			}
		}
	}
	else
	{
		// TODO : (클라이언트) 비주얼 효과나 사운드 재생 등
	}
	EndAbility(Handle, ActorInfo, ActivationInfo, true, false);
}
