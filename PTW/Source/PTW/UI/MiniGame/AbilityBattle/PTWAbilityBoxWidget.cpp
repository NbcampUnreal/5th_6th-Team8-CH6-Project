// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/MiniGame/AbilityBattle/PTWAbilityBoxWidget.h"

#include "Components/Button.h"
#include "Components/Image.h"
#include "Components/TextBlock.h"
#include "MiniGame/Data/AbilityBattle/PTWAbilityDefinition.h"
#include "MiniGame/Data/AbilityBattle/PTWAbilityRow.h"


void UPTWAbilityBoxWidget::NativeConstruct()
{
	Super::NativeConstruct();

	if (Button_AbilityButton)
	{
		Button_AbilityButton->OnClicked.AddDynamic(this, &UPTWAbilityBoxWidget::OnButtonClicked);
	}
}

void UPTWAbilityBoxWidget::InitAbilityBoxWidget(FName RowId, UDataTable* DataTable)
{
	CachedRowId = RowId;
	
	FPTWAbilityRow* Row = DataTable->FindRow<FPTWAbilityRow>(RowId, TEXT(""));
	if (!Row) return;

	UPTWAbilityDefinition* Definition = Row->AbilityDefinition.LoadSynchronous();
	if (!Definition) return;
	
	Image_AbilityIcon->SetBrushFromSoftTexture(Definition->Icon);
	Text_AbilityName->SetText(Definition->Name);
	Text_AbilityDescription->SetText(Definition->Description);
}

void UPTWAbilityBoxWidget::OnButtonClicked()
{
	OnDraftSelected.Broadcast(CachedRowId);
}
