// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/MiniGame/AbilityBattle/PTWAbilityDraftWidget.h"

#include "PTWAbilityBoxWidget.h"
#include "Components/HorizontalBox.h"
#include "Components/HorizontalBoxSlot.h"
#include "CoreFramework/PTWPlayerController.h"
#include "MiniGame/ControllerComponent/AbilityBattle/PTWAbilityControllerComponent.h"

void UPTWAbilityDraftWidget::GenerateAbilityBoxes(TArray<FName> RowId)
{
	if (!HorizontalBox)
	{
		UE_LOG(LogTemp, Error, TEXT("[DraftWidget] HorizontalBox is null"));
	}

	if (!AbilityBoxClass)
	{
		UE_LOG(LogTemp, Error, TEXT("[DraftWidget] AbilityBoxClass is null"));
	}

	if (!AbilityDraftDataTable)
	{
		UE_LOG(LogTemp, Error, TEXT("[DraftWidget] AbilityDraftDataTable is null"));
	}

	if (!HorizontalBox || !AbilityBoxClass || !AbilityDraftDataTable)
	{
		return;
	}

	
	if (RowId.IsEmpty())
	{
		UE_LOG(LogTemp, Error, TEXT("[DraftWidget] Rowid empty"));
	}
	
	for (int32 i = 0; i < RowId.Num(); i++)
	{
		UPTWAbilityBoxWidget* BoxWidget = CreateWidget<UPTWAbilityBoxWidget>(this, AbilityBoxClass);
		if (!BoxWidget) return;

		BoxWidget->OnDraftSelected.AddUObject(this, &UPTWAbilityDraftWidget::OnDraftSelected);
		BoxWidget->InitAbilityBoxWidget(RowId[i], AbilityDraftDataTable);
		UHorizontalBoxSlot* BoxSlot = HorizontalBox->AddChildToHorizontalBox(BoxWidget);

		
		FSlateChildSize FillSize;
		FillSize.SizeRule = ESlateSizeRule::Fill;
		BoxSlot->SetSize(FillSize);

		//BoxSlot->SetPadding(FMargin(30.f, 0.f));
		BoxSlot->SetHorizontalAlignment(HAlign_Center); 
		BoxSlot->SetVerticalAlignment(VAlign_Center);
	}
}

void UPTWAbilityDraftWidget::OnDraftSelected(FName RowId)
{
	if (bIsSelected) return;

	bIsSelected = true;
	
	RemoveFromParent();
	
	APTWPlayerController* PlayerController = GetOwningPlayer<APTWPlayerController>();
	if (!PlayerController) return;
	
	UPTWAbilityControllerComponent* ControllerComponent = Cast<UPTWAbilityControllerComponent>(PlayerController->GetControllerComponent());
	if (!ControllerComponent) return;

	ControllerComponent->Server_SelectedAbility(RowId);
	
}
