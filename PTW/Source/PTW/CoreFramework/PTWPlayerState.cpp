// Fill out your copyright notice in the Description page of Project Settings.


#include "PTWPlayerState.h"

#include "GAS/PTWWeaponAttributeSet.h"
#include "PTW/GAS/PTWAbilitySystemComponent.h"
#include "PTW/GAS/PTWAttributeSet.h"
#include "Net/UnrealNetwork.h"
#include "GameplayEffect.h"
#include "GameplayAbilitySpec.h"

APTWPlayerState::APTWPlayerState()
{
	NetUpdateFrequency = 100.0f;

	AbilitySystemComponent = CreateDefaultSubobject<UPTWAbilitySystemComponent>(TEXT("AbilitySystemComponent"));
	AbilitySystemComponent->SetIsReplicated(true);
	AbilitySystemComponent->SetReplicationMode(EGameplayEffectReplicationMode::Mixed);

	AttributeSet = CreateDefaultSubobject<UPTWAttributeSet>(TEXT("AttributeSet"));
	WeaponAttributeSet = CreateDefaultSubobject<UPTWWeaponAttributeSet>(TEXT("WeaponAttributeSet"));

	CurrentPlayerData.PlayerName = "";
	CurrentPlayerData.TotalWinPoints = 0;
	CurrentPlayerData.Gold = 0.0f;
}

void APTWPlayerState::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(APTWPlayerState, CurrentPlayerData);
	DOREPLIFETIME(APTWPlayerState, PlayerRoundData);
}

UAbilitySystemComponent* APTWPlayerState::GetAbilitySystemComponent() const
{
	return AbilitySystemComponent;
}

void APTWPlayerState::SetPlayerData(const FPTWPlayerData& NewData)
{
	if (HasAuthority())
	{
		CurrentPlayerData = NewData;
		OnPlayerDataUpdated.Broadcast(CurrentPlayerData);
	}
}

void APTWPlayerState::SetPlayerRoundData(const FPTWPlayerRoundData& NewData)
{
	if (HasAuthority())
	{
		PlayerRoundData = NewData;
	}
}

FPTWPlayerData APTWPlayerState::GetPlayerData() const
{
	return CurrentPlayerData;
}

FPTWPlayerRoundData APTWPlayerState::GetPlayerRoundData() const
{
	return PlayerRoundData;
}

void APTWPlayerState::InjectAbility(TSubclassOf<UGameplayAbility> AbilityClass)
{
	if (!AbilityClass || !HasAuthority()) return;

	if (!AdditionalAbilities.Contains(AbilityClass))
	{
		AdditionalAbilities.Add(AbilityClass);
	}
	if (AbilitySystemComponent)
	{
		if (!AbilitySystemComponent->FindAbilitySpecFromClass(AbilityClass))
		{
			FGameplayAbilitySpec Spec(AbilityClass, 1, INDEX_NONE, this);
			AbilitySystemComponent->GiveAbility(Spec);
		}
	}
}

void APTWPlayerState::InjectEffect(TSubclassOf<UGameplayEffect> EffectClass)
{
	if (!EffectClass || !HasAuthority()) return;

	if (!AdditionalEffects.Contains(EffectClass))
	{
		AdditionalEffects.Add(EffectClass);
	}

	if (AbilitySystemComponent)
	{
		const UGameplayEffect* EffectCDO = EffectClass->GetDefaultObject<UGameplayEffect>();

		if (EffectCDO->DurationPolicy == EGameplayEffectDurationType::Infinite)
		{

		}

		FGameplayEffectContextHandle Context = AbilitySystemComponent->MakeEffectContext();
		Context.AddSourceObject(this);
		FGameplayEffectSpecHandle Spec = AbilitySystemComponent->MakeOutgoingSpec(EffectClass, 1.0f, Context);

		if (Spec.IsValid())
		{
			AbilitySystemComponent->ApplyGameplayEffectSpecToSelf(*Spec.Data.Get());
		}
	}
}

void APTWPlayerState::ApplyAdditionalAbilities()
{
	if (!HasAuthority() || !AbilitySystemComponent) return;

	for (const TSubclassOf<UGameplayAbility>& AbilityClass : AdditionalAbilities)
	{
		if (AbilityClass)
		{
			if (!AbilitySystemComponent->FindAbilitySpecFromClass(AbilityClass))
			{
				FGameplayAbilitySpec Spec(AbilityClass, 1, INDEX_NONE, this);
				AbilitySystemComponent->GiveAbility(Spec);
			}
		}
	}
}

void APTWPlayerState::ApplyAdditionalEffects()
{
	if (!HasAuthority() || !AbilitySystemComponent) return;

	for (const TSubclassOf<UGameplayEffect>& EffectClass : AdditionalEffects)
	{
		if (!EffectClass) continue;

		const UGameplayEffect* EffectCDO = EffectClass->GetDefaultObject<UGameplayEffect>();

		 if (EffectCDO->DurationPolicy == EGameplayEffectDurationType::Infinite)
		 {
		      if (AbilitySystemComponent->HasAnyMatchingGameplayTags(EffectCDO->InheritableGameplayEffectTags.CombinedTags)) continue;
		 }

		FGameplayEffectContextHandle Context = AbilitySystemComponent->MakeEffectContext();
		Context.AddSourceObject(this);
		FGameplayEffectSpecHandle Spec = AbilitySystemComponent->MakeOutgoingSpec(EffectClass, 1.0f, Context);

		if (Spec.IsValid())
		{
			AbilitySystemComponent->ApplyGameplayEffectSpecToSelf(*Spec.Data.Get());
		}
	}
}

void APTWPlayerState::AddKillCount(int32 KillCount)
{
	if (HasAuthority())
	{
		PlayerRoundData.KillCount += KillCount;
	}
}

void APTWPlayerState::AddDeathCount(int32 DeathCount)
{
	if (HasAuthority())
	{
		PlayerRoundData.DeathCount += DeathCount;
	}
}

void APTWPlayerState::AddScore(int32 AddScore)
{
	if (HasAuthority())
	{
		PlayerRoundData.Score += AddScore;
	}
}

void APTWPlayerState::ResetRoundData()
{
	if (HasAuthority())
	{
		PlayerRoundData = FPTWPlayerRoundData();
	}
}

void APTWPlayerState::OnRep_CurrentPlayerData()
{
	OnPlayerDataUpdated.Broadcast(CurrentPlayerData);
}

void APTWPlayerState::OnRep_PlayerRoundData()
{
	OnPlayerRoundDataUpdated.Broadcast(PlayerRoundData);
}
