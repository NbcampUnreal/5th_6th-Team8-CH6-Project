// Fill out your copyright notice in the Description page of Project Settings.


#include "PTWGA_Melee.h"

#include "Abilities/Tasks/AbilityTask_PlayMontageAndWait.h"
#include "Abilities/Tasks/AbilityTask_WaitDelay.h"
#include "Abilities/Tasks/AbilityTask_WaitGameplayEvent.h"
#include "CoreFramework/PTWPlayerCharacter.h"
#include "CoreFramework/Character/Component/PTWWeaponComponent.h"
#include "PTWGameplayTag/GameplayTags.h"


void UPTWGA_Melee::ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo,
                                   const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData)
{
	Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);
	
	UAbilityTask_WaitGameplayEvent* WaitEventTask = UAbilityTask_WaitGameplayEvent::WaitGameplayEvent(this, GameplayTags::Event::Melee::Hit);
	WaitEventTask->EventReceived.AddDynamic(this, &ThisClass::OnMeleeHitReceived);
	WaitEventTask->ReadyForActivation();
	
	APTWPlayerCharacter* PC = Cast<APTWPlayerCharacter>(GetAvatarActorFromActorInfo());
	
	
	if (PC)
	{
		FGameplayTag FireTag = FGameplayTag::RequestGameplayTag(FName("Weapon.Anim.Fire"));
		UAnimMontage* MontageToPlay = PC->GetWeaponComponent()->PlayWeaponMontages(FireTag, false);

		if (MontageToPlay)
		{
			UAbilityTask_PlayMontageAndWait* MontageTask = UAbilityTask_PlayMontageAndWait::CreatePlayMontageAndWaitProxy(
				this, NAME_None, MontageToPlay);

			MontageTask->OnCompleted.AddDynamic(this, &ThisClass::K2_EndAbility);
			MontageTask->OnInterrupted.AddDynamic(this, &ThisClass::K2_EndAbility);
			MontageTask->ReadyForActivation();
		}
	}
}

void UPTWGA_Melee::OnMeleeHitReceived(FGameplayEventData Payload)
{
	if (HasAuthority(&CurrentActivationInfo)) 
	{
		if (bApplyKnockBack)
		{
			const ACharacter* Victim = Cast<ACharacter>(Payload.Target);
			ACharacter* Vic = const_cast<ACharacter*>(Victim);
			ApplyKnockBack(Vic);
		}
	}
}

void UPTWGA_Melee::ApplyKnockBack(ACharacter* Vic)
{
	AActor* Attacker = GetAvatarActorFromActorInfo();
	if (Vic && Attacker)
	{
		FVector LaunchDir = Vic->GetActorLocation() - Attacker->GetActorLocation();
		LaunchDir.Z = 0.0f; 
		LaunchDir.Normalize();
			
		float LaunchStrength = 500.0f;
		float UpwardForce = 250.0f;
		FVector FinalLaunch = (LaunchDir * LaunchStrength) + FVector(0, 0, UpwardForce);
			
		Vic->LaunchCharacter(FinalLaunch, true, true);
	}
}

