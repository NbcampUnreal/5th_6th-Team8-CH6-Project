// Fill out your copyright notice in the Description page of Project Settings.


#include "MiniGame/PlayerStateComponent/PTWAbilityBattlePSComponent.h"

#include "Net/UnrealNetwork.h"

// Sets default values for this component's properties


UPTWAbilityBattlePSComponent::UPTWAbilityBattlePSComponent()
{
	SetIsReplicatedByDefault(true);
}

void UPTWAbilityBattlePSComponent::GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ThisClass, DraftCharges);
	DOREPLIFETIME(ThisClass, bFirstDraftCompleted);
}

void UPTWAbilityBattlePSComponent::AddDraftCharges()
{
	if (GetOwner() || GetOwner()->HasAuthority())
	{
		DraftCharges++;
	}
}

void UPTWAbilityBattlePSComponent::DecreaseDraftCharges()
{
	if (GetOwner() || GetOwner()->HasAuthority())
	{
		DraftCharges--;
	}
}

void UPTWAbilityBattlePSComponent::SetCurrentDraft(const TArray<FName>& NewDraft)
{
	CurrentDraft = NewDraft;
}

void UPTWAbilityBattlePSComponent::ResetCurrentDraft()
{
}
