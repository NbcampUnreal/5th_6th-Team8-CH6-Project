// Fill out your copyright notice in the Description page of Project Settings.


#include "MiniGame/PlayerStateComponent/PTWAbilityBattlePSComponent.h"

#include "AbilitySystemComponent.h"
#include "CoreFramework/PTWPlayerState.h"
#include "Net/UnrealNetwork.h"
#include "PTWGameplayTag/GameplayTags.h"

// Sets default values for this component's properties


UPTWAbilityBattlePSComponent::UPTWAbilityBattlePSComponent()
{
	SetIsReplicatedByDefault(true);
}

void UPTWAbilityBattlePSComponent::Init(APTWPlayerState* PlayerState)
{
	ASC = Cast<APTWPlayerState>(GetOwner())->GetAbilitySystemComponent();
}

void UPTWAbilityBattlePSComponent::GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ThisClass, DraftChargeCount);
	DOREPLIFETIME(ThisClass, bFirstDraftCompleted);
	DOREPLIFETIME(ThisClass, ChargeRemainTime);
	
}

void UPTWAbilityBattlePSComponent::AddDraftCharges()
{
	if (GetOwner() && GetOwner()->HasAuthority())
	{
		DraftChargeCount++;
	}
}

void UPTWAbilityBattlePSComponent::DecreaseDraftCharges()
{
	if (GetOwner() && GetOwner()->HasAuthority())
	{
		DraftChargeCount--;
	}
}

void UPTWAbilityBattlePSComponent::SetCurrentDraft(const TArray<FName>& NewDraft)
{
	CurrentDraft = NewDraft;
}

void UPTWAbilityBattlePSComponent::ResetCurrentDraft()
{
	CurrentDraft.Reset();
}

void UPTWAbilityBattlePSComponent::UpdateChargeRemainTime(float DecreaseTimer)
{
	if (!GetOwner() || !GetOwner()->HasAuthority()) return;

	APTWPlayerState* PlayerState = Cast<APTWPlayerState>(GetOwner());
	if (!PlayerState) return;
	
	if (!ASC) return;
	
	if (ASC->HasMatchingGameplayTag(GameplayTags::State::Status_Dead)) return;
	ChargeRemainTime -= DecreaseTimer;

	if (ChargeRemainTime <= 0.f)
	{
		ChargeRemainTime = MaxChargeTime;
		AddDraftCharges();
	}

	//UE_LOG(LogTemp, Log, TEXT("ChargeRemainTime: %f"), ChargeRemainTime);
}

void UPTWAbilityBattlePSComponent::ApplyHealthRegenEffect()
{
	if (!ASC || !HealthRegenEffect) return;

	FGameplayEffectContextHandle Context = ASC->MakeEffectContext();

	FGameplayEffectSpecHandle SpecHandle = ASC->MakeOutgoingSpec(
		HealthRegenEffect,
		1.0f,
		Context
	);

	if (SpecHandle.IsValid())
	{
		ASC->ApplyGameplayEffectSpecToSelf(*SpecHandle.Data.Get());
	}
}

void UPTWAbilityBattlePSComponent::OnRep_ChargeRemainTime()
{
	OnDraftChargedTimeChanged.Broadcast(ChargeRemainTime, MaxChargeTime);
	
}

void UPTWAbilityBattlePSComponent::OnRep_ChangeChargeCount()
{
	OnChangedChargeCount.Broadcast(DraftChargeCount);
}


