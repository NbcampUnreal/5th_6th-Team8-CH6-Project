// Fill out your copyright notice in the Description page of Project Settings.


#include "MiniGame/Manager/PTWChaosEventApply.h"

#include "GameFramework/PlayerState.h"
#include "AbilitySystemComponent.h"
#include "AbilitySystemInterface.h"
#include "CoreFramework/Game/GameState/PTWGameState.h"
#include "MiniGame/Data/PTWChaosItemDefinition.h"
#include "PTWGameplayTag/GameplayTags.h"


void UPTWChaosEventApply::InitDefinition(UPTWChaosItemDefinition* InDefinition)
{
	Definition = InDefinition;
}

void UPTWChaosEventApply::SetStackCount(int32 Count)
{
	StackCount = Count;
}

void UPTWChaosEventApply::ApplyChaosEffect(APTWGameState* GameState)
{
	if (!IsValid(GameState) || !IsValid(Definition) || !Definition->GameplayEffectClass) return;

	for (APlayerState* PlayerState : GameState->PlayerArray)
	{
		if (!PlayerState) continue;

		UAbilitySystemComponent* ASC = nullptr;

		if (IAbilitySystemInterface* ASI = Cast<IAbilitySystemInterface>(PlayerState))
		{
			ASC = ASI->GetAbilitySystemComponent();
		}
		
		if (!IsValid(ASC)) continue;

		FGameplayEffectContextHandle ContextHandle = ASC->MakeEffectContext();
		
		int32 Level = Definition->bUseStack ? StackCount : 1;
		
		FGameplayEffectSpecHandle SpecHandle = ASC->MakeOutgoingSpec(
			Definition->GameplayEffectClass, Level, ContextHandle);
		
		if (!SpecHandle.IsValid()) continue;
		
		FActiveGameplayEffectHandle ActiveHandle = ASC->ApplyGameplayEffectSpecToSelf(*SpecHandle.Data.Get());

		if (!ActiveHandle.IsValid()) continue;

		ApplyEffectHandles.Add(ASC, ActiveHandle);
	}
}

void UPTWChaosEventApply::ApplyChaosEvent(APTWGameState* GameState)
{
	if (Definition->GameplayEffectClass)
	{
		ApplyChaosEffect(GameState);
	}
}

void UPTWChaosEventApply::ChaosEventEnd()
{
	ChaosEffectEnd();
}

void UPTWChaosEventApply::ChaosEffectEnd()
{
	for (auto pair : ApplyEffectHandles)
	{
		if (!IsValid(pair.Key)) continue;
		if (!pair.Value.IsValid()) continue;
			
		UAbilitySystemComponent* ASC = pair.Key;
		FActiveGameplayEffectHandle ActiveHandle = pair.Value;

		ASC->RemoveActiveGameplayEffect(ActiveHandle);
	}
	ApplyEffectHandles.Empty();
}
