// Fill out your copyright notice in the Description page of Project Settings.


#include "PTWGA_RBLeaderSlower.h"

#include "CoreFramework/PTWCombatInterface.h"
#include "CoreFramework/PTWPlayerCharacter.h"
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
	
	// APTWPlayerCharacter* MyPC = Cast<APTWPlayerCharacter>(GetAvatarActorFromActorInfo());
	// if (!MyPC) return;
	//
	// APTWPlayerController* MyController = Cast<APTWPlayerController>(MyPC->GetController());
	// if (!MyController) return;
	//
	// if (MyController == LeaderController) return;
	
	ApplyEffectToTarget(LeaderController);
	SendActivateMsg(LeaderController);
	EndAbility(Handle, ActorInfo, ActivationInfo, false, false);
}

void UPTWGA_RBLeaderSlower::ApplyEffectToTarget(APTWPlayerController* TargetController)
{
	IPTWCombatInterface* CombatInterface = Cast<IPTWCombatInterface>(TargetController->GetPawn());
	if (!CombatInterface) return;
	
	CombatInterface->ApplyGameplayEffectToSelf(SlowEffect, 1.0f, FGameplayEffectContextHandle());
}

void UPTWGA_RBLeaderSlower::SendActivateMsg(APTWPlayerController* TargetController)
{
	APTWPlayerCharacter* MyCharacter =  Cast<APTWPlayerCharacter>(GetAvatarActorFromActorInfo());
	if (!MyCharacter) return;
	
	APTWPlayerController* MyController = Cast<APTWPlayerController>(MyCharacter->GetController());
	if (!MyController) return;
	
#define LOCTEXT_NAMESPACE "DeliveryGameMode"
	FText DeliveryItemLeaderSlowMessage = LOCTEXT("GetLeaderSlower!", "Leader Slowed by an enemy item!");
#undef LOCTEXT_NAMESPACE
	TargetController->SendMessage(DeliveryItemLeaderSlowMessage, ENotificationPriority::Normal, 3);
}
