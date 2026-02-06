// Fill out your copyright notice in the Description page of Project Settings.


#include "PTWBombAttributeSet.h"
#include "Net/UnrealNetwork.h"
#include "GameFramework/Actor.h"
#include "MiniGame/Item/PTWBombActor.h"

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

void UPTWBombAttributeSet::PostAttributeBaseChange(
	const FGameplayAttribute& Attribute,
	float OldValue,
	float NewValue
) const
{
	Super::PostAttributeBaseChange(Attribute, OldValue, NewValue);
	
	if (Attribute != GetRemainingTimeAttribute())
	{
		return;
	}

	//  0 이하로 떨어졌을 때만 발동
	if (NewValue > 0.f)
	{
		return;
	}

	AActor* Owner = GetOwningActor();
	if (!Owner) return;
	
	if (!Owner->HasAuthority())
	{
		return;
	}
	
	if (APTWBombActor* BombActor = Cast<APTWBombActor>(Owner))
	{
		AActor* InstigatorActor = BombActor->GetBombOwnerPawn() ? BombActor->GetBombOwnerPawn() : BombActor;
		BombActor->RequestExplode(InstigatorActor);
	}

}

