// Fill out your copyright notice in the Description page of Project Settings.


#include "PTWGA_RBLeaderSlower.h"

#include "CoreFramework/PTWCombatInterface.h"
#include "CoreFramework/PTWPlayerController.h"
#include "MiniGame/GameMode/PTWDeliveryGameMode.h"

void UPTWGA_RBLeaderSlower::ActivateAbility(const FGameplayAbilitySpecHandle Handle,
                                            const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo,
                                            const FGameplayEventData* TriggerEventData)
{
	Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);
	
	if (!HasAuthority(&CurrentActivationInfo)) return;
	APTWDeliveryGameMode* DeliveryGameMode = Cast<APTWDeliveryGameMode>(GetWorld()->GetAuthGameMode());

	if (!DeliveryGameMode) return;
	
	APTWPlayerController* LeaderController = DeliveryGameMode->GetLeaderController();
	if (!LeaderController) return;
	
	IPTWCombatInterface* CombatInterface = Cast<IPTWCombatInterface>(LeaderController->GetPawn());
	if (!CombatInterface) return;
	
	CombatInterface->ApplyGameplayEffectToSelf(SlowEffect, 1.0f, FGameplayEffectContextHandle());
	
	EndAbility(Handle, ActorInfo, ActivationInfo, false, false);
}
