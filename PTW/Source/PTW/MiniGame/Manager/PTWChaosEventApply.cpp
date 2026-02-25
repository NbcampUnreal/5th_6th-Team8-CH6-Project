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

void UPTWChaosEventApply::InitHandles()
{
	ChaosHandle.Add(GameplayTags::Item::Chaos::Test, [this]() {Test();});
}

void UPTWChaosEventApply::Test()
{
	//GEngine->AddOnScreenDebugMessage(0, 10.f, )
}

void UPTWChaosEventApply::ApplyChaosEffect(APTWGameState* GameState)
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

void UPTWChaosEventApply::ChaosEventApply(APTWGameState* GameState)
{
	if (Definition->GameplayEffectClass)
	{
		ApplyChaosEffect(GameState);
	}

	if (auto* Handle = ChaosHandle.Find(Definition->ItemTag))
	{
		(*Handle)();
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
