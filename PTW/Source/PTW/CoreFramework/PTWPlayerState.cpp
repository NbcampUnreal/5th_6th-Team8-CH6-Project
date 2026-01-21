// Fill out your copyright notice in the Description page of Project Settings.


#include "PTWPlayerState.h"
#include "PTW/GAS/PTWAbilitySystemComponent.h"
#include "PTW/GAS/PTWAttributeSet.h"

APTWPlayerState::APTWPlayerState()
{
	NetUpdateFrequency = 100.0f;

	AbilitySystemComponent = CreateDefaultSubobject<UPTWAbilitySystemComponent>(TEXT("AbilitySystemComponent"));
	AbilitySystemComponent->SetIsReplicated(true);
	AbilitySystemComponent->SetReplicationMode(EGameplayEffectReplicationMode::Mixed);

	AttributeSet = CreateDefaultSubobject<UPTWAttributeSet>(TEXT("AttributeSet"));
}

UAbilitySystemComponent* APTWPlayerState::GetAbilitySystemComponent() const
{
	return AbilitySystemComponent;
}
