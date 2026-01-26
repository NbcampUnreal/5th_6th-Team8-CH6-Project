// Fill out your copyright notice in the Description page of Project Settings.


#include "PTWGameplayAbility.h"
#include "PTW/CoreFramework/PTWBaseCharacter.h"
#include "PTW/CoreFramework/PTWPlayerCharacter.h"
#include "PTW/CoreFramework/PTWPlayerController.h"

UPTWGameplayAbility::UPTWGameplayAbility()
{
	InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;
}

APTWBaseCharacter* UPTWGameplayAbility::GetPTWCharacterFromActorInfo() const
{
	return Cast<APTWBaseCharacter>(GetAvatarActorFromActorInfo());
}

APTWPlayerCharacter* UPTWGameplayAbility::GetPTWPlayerCharacterFromActorInfo() const
{
	return Cast<APTWPlayerCharacter>(GetAvatarActorFromActorInfo());
}

APTWPlayerController* UPTWGameplayAbility::GetPTWPlayerControllerFromActorInfo() const
{
	if (CurrentActorInfo->PlayerController.IsValid())
	{
		return Cast<APTWPlayerController>(CurrentActorInfo->PlayerController.Get());
	}
	return nullptr;
}
