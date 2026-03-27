// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/MiniGame/AbilityBattle/PTWAbilityDraftWidget.h"

#include "AbilitySystemComponent.h"
#include "PTWAbilityBoxWidget.h"
#include "PTWDraftCharge.h"
#include "PTWShieldBar.h"
#include "Components/HorizontalBox.h"
#include "Components/HorizontalBoxSlot.h"
#include "CoreFramework/PTWPlayerController.h"
#include "CoreFramework/PTWPlayerState.h"
#include "GAS/PTWAttributeSet.h"
#include "MiniGame/ControllerComponent/AbilityBattle/PTWAbilityControllerComponent.h"
#include "MiniGame/PlayerStateComponent/PTWAbilityBattlePSComponent.h"

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
	
	HorizontalBox->ClearChildren();
	
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
	
	APTWPlayerController* PTWPlayerController = Cast<APTWPlayerController>(GetOwningPlayer());
	if (!PTWPlayerController) return;

	UPTWAbilityBattlePSComponent* PlayerStateComponent = Cast<UPTWAbilityBattlePSComponent>(PTWPlayerController->GetPlayerState<APTWPlayerState>()->GetMiniGameComponent());
	if (!PlayerStateComponent) return;
	
	UActorComponent* ActorComponent = PTWPlayerController->GetControllerComponent();
	if (!ActorComponent)
	{
		//UE_LOG(LogTemp, Warning, TEXT("[DraftWidget] ActorComponent Class: %s"),*ActorComponent->GetClass()->GetName());
		return;
	}
	
	UPTWAbilityControllerComponent* ControllerComponent = Cast<UPTWAbilityControllerComponent>(ActorComponent);
	if (!ControllerComponent) return;
	
	ControllerComponent->Server_SelectedAbility(RowId);
	ControllerComponent->Client_HideDraftUI();
}

void UPTWAbilityDraftWidget::OnShieldChanged(const FOnAttributeChangeData& Data)
{
	const float Current = Data.NewValue;
	
	WBP_ShieldBar->SetCurrentShield(Current);
}

void UPTWAbilityDraftWidget::OnMaxShieldChanged(const FOnAttributeChangeData& Data)
{
	const float Current = Data.NewValue;
	
	WBP_ShieldBar->SetCurrentMaxShield(Current);

	if (!WBP_ShieldBar->IsVisible())
	{
		WBP_ShieldBar->SetVisibility(ESlateVisibility::Visible);
	}
}

void UPTWAbilityDraftWidget::NativeConstruct()
{
	Super::NativeConstruct();

	APlayerController* PlayerController = GetOwningPlayer();
	if (!PlayerController) return;

	APTWPlayerState* PlayerState = PlayerController->GetPlayerState<APTWPlayerState>();
	if (!PlayerState) return;

	UAbilitySystemComponent* ASC= PlayerState->GetAbilitySystemComponent();
	if (!ASC) return;
	
	UPTWAbilityBattlePSComponent* PSComponent = Cast<UPTWAbilityBattlePSComponent>(PlayerState->GetMiniGameComponent());
	if (!PSComponent) return;

	WBP_ShieldBar->SetVisibility(ESlateVisibility::Hidden);
	
	PSComponent->OnChangedChargeCount.AddUObject(WBP_DraftCharge, &UPTWDraftCharge::UpdateChargeCount);
	PSComponent->OnDraftChargedTimeChanged.AddUObject(WBP_DraftCharge, &UPTWDraftCharge::UpdateChargeTime);

	ASC->GetGameplayAttributeValueChangeDelegate(UPTWAttributeSet::GetShieldAttribute()).AddUObject(this, &UPTWAbilityDraftWidget::OnShieldChanged);
	ASC->GetGameplayAttributeValueChangeDelegate(UPTWAbilityBattleAttributeSet::GetMaxShieldAttribute()).AddUObject(this, &UPTWAbilityDraftWidget::OnMaxShieldChanged);

	
}
