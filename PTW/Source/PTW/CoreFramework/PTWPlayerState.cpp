// Fill out your copyright notice in the Description page of Project Settings.


#include "PTWPlayerState.h"

#include "GAS/PTWWeaponAttributeSet.h"
#include "PTW/GAS/PTWAbilitySystemComponent.h"
#include "PTW/GAS/PTWAttributeSet.h"
#include "Net/UnrealNetwork.h"

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

FPTWPlayerData APTWPlayerState::GetPlayerData() const
{
	return CurrentPlayerData;
}

void APTWPlayerState::OnRep_CurrentPlayerData()
{
	OnPlayerDataUpdated.Broadcast(CurrentPlayerData);
}
