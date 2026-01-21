// Fill out your copyright notice in the Description page of Project Settings.


#include "PTWAbilitySystemComponent.h"
#include "Abilities/GameplayAbility.h"

void UPTWAbilitySystemComponent::Input_Pressed(const FGameplayTag& InputTag)
{
	if (!InputTag.IsValid()) return;

	for (FGameplayAbilitySpec& Spec : ActivatableAbilities.Items)
	{

		if (Spec.Ability && Spec.DynamicAbilityTags.HasTagExact(InputTag))
		{
			AbilitySpecInputPressed(Spec);

			if (!Spec.IsActive())
			{
				TryActivateAbility(Spec.Handle);
			}
		}
	}
}

void UPTWAbilitySystemComponent::Input_Released(const FGameplayTag& InputTag)
{
	if (!InputTag.IsValid()) return;

	for (FGameplayAbilitySpec& Spec : ActivatableAbilities.Items)
	{
		if (Spec.Ability && Spec.DynamicAbilityTags.HasTagExact(InputTag))
		{
			AbilitySpecInputReleased(Spec);
		}
	}
}
