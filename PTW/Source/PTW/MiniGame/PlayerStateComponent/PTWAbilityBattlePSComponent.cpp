// Fill out your copyright notice in the Description page of Project Settings.


#include "MiniGame/PlayerStateComponent/PTWAbilityBattlePSComponent.h"

// Sets default values for this component's properties


UPTWAbilityBattlePSComponent::UPTWAbilityBattlePSComponent()
{
	SetIsReplicatedByDefault(true);
}

void UPTWAbilityBattlePSComponent::AddDraftCharges()
{
}

void UPTWAbilityBattlePSComponent::DecreaseDraftCharges()
{
}

void UPTWAbilityBattlePSComponent::SetCurrentDraft(const TArray<FName>& NewDraft)
{
	CurrentDraft = NewDraft;
}

void UPTWAbilityBattlePSComponent::ResetCurrentDraft()
{
}
