// Fill out your copyright notice in the Description page of Project Settings.


#include "PTWBombAttributeSet.h"
#include "Net/UnrealNetwork.h"

UPTWBombAttributeSet::UPTWBombAttributeSet()
{
	RemainingTime = 0.f; 
}

void UPTWBombAttributeSet::OnRep_RemainingTime(const FGameplayAttributeData& OldValue)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UPTWBombAttributeSet, RemainingTime, OldValue);
}

void UPTWBombAttributeSet::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME_CONDITION_NOTIFY(UPTWBombAttributeSet, RemainingTime, COND_None, REPNOTIFY_Always);
}

void UPTWBombAttributeSet::PreAttributeChange(const FGameplayAttribute& Attribute, float& NewValue)
{
	Super::PreAttributeChange(Attribute, NewValue);

	// RemainingTime이 0 밑으로 X
	if (Attribute == GetRemainingTimeAttribute())
	{
		NewValue = FMath::Max(NewValue, 0.f);
	}
}

void UPTWBombAttributeSet::PostAttributeBaseChange(const FGameplayAttribute& Attribute, float OldValue,
	float NewValue) const
{
	Super::PostAttributeBaseChange(Attribute, OldValue, NewValue);
	
	if (GetRemainingTime() == 0.f)
	{
		// 폭탄 터짐 로직
		AActor* Owner = GetOwningActor();
		
		// 폭탄 액터 -> Explode
		// 폭탄 액터 클래스로 캐스팅 후 함수 호출 
	}
}


