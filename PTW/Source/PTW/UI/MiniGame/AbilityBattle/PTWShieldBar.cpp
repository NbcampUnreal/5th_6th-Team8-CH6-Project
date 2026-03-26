// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/MiniGame/AbilityBattle/PTWShieldBar.h"

#include "FindInBlueprints.h"
#include "Components/ProgressBar.h"

void UPTWShieldBar::SetProgressBarPer(float Current, float Max)
{
	float Per = (Current / Max);

	ProgressBar_Shield->SetPercent(Per);
}
