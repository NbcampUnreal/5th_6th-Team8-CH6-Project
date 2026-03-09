// Fill out your copyright notice in the Description page of Project Settings.


#include "GAS/Abilities/ItemAbilities/ChaosItem/PTWGiantPotion.h"

#include "GameFramework/Character.h"

void UPTWGiantPotion::ActivateAbility(const FGameplayAbilitySpecHandle Handle,
                                      const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo,
                                      const FGameplayEventData* TriggerEventData)
{
	Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);

	ACharacter* Character = Cast<ACharacter>(ActorInfo->AvatarActor.Get());
	if (!Character || !Character->HasAuthority()) return;

	OriginalScale = Character->GetActorScale3D();

	float Scale = FMath::Clamp((GetAbilityLevel() * 0.1), 0.1f, 1.f) ;
	Character->SetActorScale3D(OriginalScale * Scale);
}

void UPTWGiantPotion::EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo,
	const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled)
{
	Super::EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);

	ACharacter* Character = Cast<ACharacter>(ActorInfo->AvatarActor.Get());
	if (!Character || !Character->HasAuthority()) return;

	Character->SetActorScale3D(OriginalScale);
}
