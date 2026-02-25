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
		PC->GetWeaponComponent()->PlayMontage1P(MeleeAttackMontage);
		PC->GetMesh3P()->GetAnimInstance()->Montage_Play(MeleeAttackMontage);
	}
	
	float Duration = MeleeAttackMontage->GetPlayLength();
	UAbilityTask_WaitDelay* DelayTask = UAbilityTask_WaitDelay::WaitDelay(this, Duration);
	DelayTask->OnFinish.AddDynamic(this, &ThisClass::K2_EndAbility);
	DelayTask->ReadyForActivation();
}

void UPTWGA_Melee::OnMeleeHitReceived(FGameplayEventData Payload)
{
	if (HasAuthority(&CurrentActivationInfo)) 
	{
		UE_LOG(LogTemp, Warning, TEXT("Hit"));
	}
}

