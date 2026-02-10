// Fill out your copyright notice in the Description page of Project Settings.


#include "MiniGame/Manager/PTWChaosEventApply.h"

#include "GameFramework/PlayerState.h"
#include "AbilitySystemComponent.h"
#include "AbilitySystemInterface.h"
#include "CoreFramework/Game/GameState/PTWGameState.h"
#include "MiniGame/Data/PTWChaosEventDefinition.h"


void UPTWChaosEventApply::InitDefinition(UPTWChaosEventDefinition* InDefinition)
{
	Definition = InDefinition;
}

void UPTWChaosEventApply::ChaosEventApply(APTWGameState* GameState)
{
	if (!IsValid(GameState) || !IsValid(Definition)) return;

	for (APlayerState* PlayerState : GameState->PlayerArray)
	{
		if (!PlayerState) continue;

		UAbilitySystemComponent* ASC = nullptr;

		if (IAbilitySystemInterface* ASI = Cast<IAbilitySystemInterface>(PlayerState))
		{
			ASC = ASI->GetAbilitySystemComponent();
		}
		
		if (!IsValid(ASC) || !Definition->GameplayEffectClass) continue;

		FGameplayEffectContextHandle ContextHandle = ASC->MakeEffectContext();
        
		FGameplayEffectSpecHandle SpecHandle = ASC->MakeOutgoingSpec(
			Definition->GameplayEffectClass, 1, ContextHandle);
		
		if (!SpecHandle.IsValid()) continue;
		
		FActiveGameplayEffectHandle ActiveHandle = ASC->ApplyGameplayEffectSpecToSelf(*SpecHandle.Data.Get());

		if (!ActiveHandle.IsValid()) return;

		ApplyEffectHandles.Add(ASC, ActiveHandle);
	}
}

void UPTWChaosEventApply::ChaosEventEnd()
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
