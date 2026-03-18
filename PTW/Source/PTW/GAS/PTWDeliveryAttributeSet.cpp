// Fill out your copyright notice in the Description page of Project Settings.


#include "PTWDeliveryAttributeSet.h"

#include "AbilitySystemBlueprintLibrary.h"
#include "GameplayEffectExtension.h"
#include "CoreFramework/PTWPlayerCharacter.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Net/UnrealNetwork.h"

UPTWDeliveryAttributeSet::UPTWDeliveryAttributeSet()
{
}

void UPTWDeliveryAttributeSet::GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME_CONDITION_NOTIFY(UPTWDeliveryAttributeSet, BatteryLevel, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UPTWDeliveryAttributeSet, MaxBatteryLevel, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UPTWDeliveryAttributeSet, MoveSpeedModifier, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UPTWDeliveryAttributeSet, ChargeSpeed, COND_None, REPNOTIFY_Always);
}

void UPTWDeliveryAttributeSet::PreAttributeChange(const FGameplayAttribute& Attribute, float& NewValue)
{
	Super::PreAttributeChange(Attribute, NewValue);
	
	if (Attribute == GetBatteryLevelAttribute())
	{
		NewValue = FMath::Clamp(NewValue, 0, 1);
	}
}

void UPTWDeliveryAttributeSet::PostGameplayEffectExecute(const FGameplayEffectModCallbackData& Data)
{
	Super::PostGameplayEffectExecute(Data);
	
	if (Data.EvaluatedData.Attribute == GetBatteryLevelAttribute())
	{
		SetBatteryLevel(FMath::Clamp(GetBatteryLevel(), 0.0f, 1.0f));
		
		float BatteryPercent = GetBatteryLevel();
		float NewSpeedModifier = 0.5f + (BatteryPercent * 0.5f); 
        
		SetMoveSpeedModifier(NewSpeedModifier);
		
		float FinalSpeed = 600.0f * NewSpeedModifier;
		
		if (UAbilitySystemComponent* ASC = GetOwningAbilitySystemComponent())
		{
			ASC->SetNumericAttributeBase(GetMoveSpeedAttribute(), FinalSpeed);
		}
	}
}

void UPTWDeliveryAttributeSet::PostAttributeChange(const FGameplayAttribute& Attribute, float OldValue, float NewValue)
{
	Super::PostAttributeChange(Attribute, OldValue, NewValue);
	
	if (Attribute == GetMoveSpeedModifierAttribute())
	{
		UpdateCharacterSpeed();
	}
}

void UPTWDeliveryAttributeSet::OnRep_BatteryLevel(const FGameplayAttributeData& OldBatteryLevel)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UPTWDeliveryAttributeSet, BatteryLevel, OldBatteryLevel);
}

void UPTWDeliveryAttributeSet::OnRep_MaxBatteryLevel(const FGameplayAttributeData& OldMaxBatteryLevel)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UPTWDeliveryAttributeSet, MaxBatteryLevel, OldMaxBatteryLevel);
}

void UPTWDeliveryAttributeSet::OnRep_ChargeSpeed(const FGameplayAttributeData& OldChargeSpeed)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UPTWDeliveryAttributeSet, ChargeSpeed, OldChargeSpeed);
}

void UPTWDeliveryAttributeSet::OnRep_MoveSpeedModifier(const FGameplayAttributeData& OldMoveSpeedMod)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UPTWDeliveryAttributeSet, MoveSpeedModifier, OldMoveSpeedMod);
}

void UPTWDeliveryAttributeSet::UpdateCharacterSpeed()
{
	if (APTWPlayerCharacter* PC = Cast<APTWPlayerCharacter>(GetOwningActor()))
	{
		if (UCharacterMovementComponent* Comp = PC->GetCharacterMovement())
		{
			Comp->MaxWalkSpeed = 600.0f * GetMoveSpeedModifier();
		}
	}
}
