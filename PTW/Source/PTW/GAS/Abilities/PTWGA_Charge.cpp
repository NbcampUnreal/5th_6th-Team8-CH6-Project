// Fill out your copyright notice in the Description page of Project Settings.


#include "PTWGA_Charge.h"

#include "AbilitySystemBlueprintLibrary.h"
#include "CoreFramework/PTWCombatInterface.h"
#include "GAS/PTWDeliveryAttributeSet.h"
#include "PTWGameplayTag/GameplayTags.h"

void UPTWGA_Charge::ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo,
                                    const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData)
{
	ApplyChargeEffect();
	GetWorld()->GetTimerManager().SetTimer(RechargeTimerHandle, this, &UPTWGA_Charge::TickReCharge, 0.05f, true);
}

void UPTWGA_Charge::ApplyChargeEffect()
{
	IPTWCombatInterface* CombatInterface = Cast<IPTWCombatInterface>(GetAvatarActorFromActorInfo());
	if (!CombatInterface) return;
	
	CombatInterface->ApplyGameplayEffectToSelf(ReChargeGEClass, 1.0f, FGameplayEffectContextHandle());
}

void UPTWGA_Charge::TickReCharge()
{
	UAbilitySystemComponent* ASC = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(GetAvatarActorFromActorInfo());
	
	const UPTWDeliveryAttributeSet* DeliveryAS = Cast<UPTWDeliveryAttributeSet>(ASC->GetAttributeSet(UPTWDeliveryAttributeSet::StaticClass()));
	if (!DeliveryAS) return;
	
	UPTWDeliveryAttributeSet* DAS = const_cast<UPTWDeliveryAttributeSet*>(DeliveryAS);
	
	float CurrentBattery = DAS->GetBatteryLevel();
	float MaxBatteryLevel = 1.0f;
	
	float NewBattery = FMath::FInterpTo(CurrentBattery, MaxBatteryLevel, 0.05f, RechargeSpeed);
	DAS->SetBatteryLevel(NewBattery);
	
	if (NewBattery >= MaxBatteryLevel - 0.01f)
	{
		DAS->SetBatteryLevel(MaxBatteryLevel);
		FinishRecharge();
	}
}

void UPTWGA_Charge::FinishRecharge()
{
	GetWorld()->GetTimerManager().ClearTimer(RechargeTimerHandle);
	OnRechargeCompleted(); 
	EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, false, false);
}

void UPTWGA_Charge::OnRechargeCompleted()
{
	IPTWCombatInterface* CombatInterface = Cast<IPTWCombatInterface>(GetAvatarActorFromActorInfo());
	if (!CombatInterface) return;
	
	CombatInterface->RemoveEffectWithTag(GameplayTags::State::Charge);
	CombatInterface->ApplyGameplayEffectToSelf(ChargeCompleteGEClass, 1.0f, FGameplayEffectContextHandle());
}
