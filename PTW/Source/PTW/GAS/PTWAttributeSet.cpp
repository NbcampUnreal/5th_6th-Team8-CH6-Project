// Fill out your copyright notice in the Description page of Project Settings.

#include "PTW/GAS/PTWAttributeSet.h"
#include "Net/UnrealNetwork.h"
#include "GameplayEffectExtension.h"


UPTWAttributeSet::UPTWAttributeSet()
{
	InitHealth(100.0f);
	InitMaxHealth(100.0f);
}

void UPTWAttributeSet::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME_CONDITION_NOTIFY(UPTWAttributeSet, Health, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UPTWAttributeSet, MaxHealth, COND_None, REPNOTIFY_Always);
}


void UPTWAttributeSet::PreAttributeChange(const FGameplayAttribute& Attribute, float& NewValue)
{
	Super::PreAttributeChange(Attribute, NewValue);

	if (Attribute == GetHealthAttribute())
	{
		NewValue = FMath::Clamp(NewValue, 0.0f, GetMaxHealth());
	}
}

void UPTWAttributeSet::PostGameplayEffectExecute(const FGameplayEffectModCallbackData& Data)
{
	Super::PostGameplayEffectExecute(Data);

	if (Data.EvaluatedData.Attribute == GetHealthAttribute())
	{
		SetHealth(FMath::Clamp(GetHealth(), 0.0f, GetMaxHealth()));

		if (GetHealth() <= 0.0f)
		{

		}
	}
}

void UPTWAttributeSet::OnRep_Health(const FGameplayAttributeData& OldHealth) { GAMEPLAYATTRIBUTE_REPNOTIFY(UPTWAttributeSet, Health, OldHealth); }
void UPTWAttributeSet::OnRep_MaxHealth(const FGameplayAttributeData& OldMaxHealth) { GAMEPLAYATTRIBUTE_REPNOTIFY(UPTWAttributeSet, MaxHealth, OldMaxHealth); }
