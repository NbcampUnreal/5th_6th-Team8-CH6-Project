// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/MiniGame/AbilityBattle/PTWDraftCharge.h"

#include "Components/Image.h"
#include "Components/TextBlock.h"


void UPTWDraftCharge::UpdateChargeTime(float RemainTime, float MaxTime)
{
	Percent = 1 - (RemainTime / MaxTime);
	
	if (ProgressMaterial)
	{
		ProgressMaterial->SetScalarParameterValue(TEXT("Fill%"), Percent);
	}
}

void UPTWDraftCharge::UpdateChargeCount(int32 Count)
{
	FText CountText  = FText::FromString(FString::FromInt(Count));
	Text_ChargeCount->SetText(CountText);
}

void UPTWDraftCharge::NativeConstruct()
{
	Super::NativeConstruct();

	ProgressMaterial = Cast<UMaterialInstanceDynamic>(Image_Progress->GetDynamicMaterial());
	
	
}
