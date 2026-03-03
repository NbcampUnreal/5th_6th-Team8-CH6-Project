// Fill out your copyright notice in the Description page of Project Settings.


#include "MiniGame/Manager/PTWChaosEventApply.h"

#include "GameFramework/PlayerState.h"
#include "AbilitySystemComponent.h"
#include "AbilitySystemInterface.h"
#include "CoreFramework/Game/GameState/PTWGameState.h"
#include "MiniGame/Data/PTWChaosItemDefinition.h"


void UPTWChaosEventApply::InitDefinition(UPTWChaosItemDefinition* InDefinition)
{
	Definition = InDefinition;
}

void UPTWChaosEventApply::SetStackCount(int32 Count)
{
	StackCount = Count;
}


void UPTWChaosEventApply::ApplyChaosEvent(APTWGameState* GameState)
{
	if (Definition->ChaosAbilityClass)
	{
		ApplyChaosAbilityClass(GameState);
	}
}

void UPTWChaosEventApply::ChaosEventEnd()
{
	ChaosAbilityEnd();
}

void UPTWChaosEventApply::ApplyChaosAbilityClass(APTWGameState* GameState)
{
	if (!IsValid(Definition) || !Definition->ChaosAbilityClass) return;

	for (APlayerState* PlayerState : GameState->PlayerArray)
	{
		UAbilitySystemComponent* ASC = nullptr;
		if (IAbilitySystemInterface* ASI = Cast<IAbilitySystemInterface>(PlayerState))
		{
			ASC = ASI->GetAbilitySystemComponent();
		}
		if (!IsValid(ASC)) continue;

		FGameplayAbilitySpec Spec(Definition->ChaosAbilityClass, StackCount);
		FGameplayAbilitySpecHandle Handle = ASC->GiveAbility(Spec);
		ASC->TryActivateAbility(Handle);
        
		ApplyAbilityHandles.Add(ASC, Handle);
	}
}

void UPTWChaosEventApply::ChaosAbilityEnd()
{
	for (auto& Pair : ApplyAbilityHandles)
	{
		if (!IsValid(Pair.Key)) continue;
		
		Pair.Key->CancelAbilityHandle(Pair.Value);
		Pair.Key->ClearAbility(Pair.Value); 
	}
	ApplyAbilityHandles.Empty();
}

