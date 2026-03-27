// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/MiniGame/AbilityBattle/PTWShieldBar.h"

#include "Components/ProgressBar.h"

void UPTWShieldBar::SetProgressBarPer()
{
	float Per = (CurrentShield / CurrentMaxShield);

	ProgressBar_Shield->SetPercent(Per);
}

void UPTWShieldBar::SetCurrentShield(float NewShield)
{
	CurrentShield = NewShield;

	SetProgressBarPer();
}

void UPTWShieldBar::SetCurrentMaxShield(float NewMaxShield)
{
	CurrentMaxShield = NewMaxShield;

	SetProgressBarPer();
}


