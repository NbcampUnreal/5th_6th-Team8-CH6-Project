// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/InGameUI/PTWCrosshair.h"
#include "AbilitySystemComponent.h"
#include "PTWGameplayTag/GameplayTags.h"

void UPTWCrosshair::InitWithASC(UAbilitySystemComponent* InASC)
{
	if (!InASC) return;
	CachedASC = InASC;

	EquipTag = GameplayTags::Weapon::State::Equip;
	SprintTag = GameplayTags::State::Movement::Sprinting;

	// 델리게이트 등록
	EquipTagHandle = CachedASC->RegisterGameplayTagEvent(EquipTag, EGameplayTagEventType::NewOrRemoved)
		.AddUObject(this, &UPTWCrosshair::OnStateTagChanged);

	SprintTagHandle = CachedASC->RegisterGameplayTagEvent(SprintTag, EGameplayTagEventType::NewOrRemoved)
		.AddUObject(this, &UPTWCrosshair::OnStateTagChanged);

	// 초기 상태 반영
	UpdateVisibility();
}

void UPTWCrosshair::NativeConstruct()
{
	Super::NativeConstruct();
	SetVisibility(ESlateVisibility::Hidden);
}

void UPTWCrosshair::NativeDestruct()
{
	if (CachedASC)
	{
		CachedASC->UnregisterGameplayTagEvent(EquipTagHandle, EquipTag, EGameplayTagEventType::NewOrRemoved);
		CachedASC->UnregisterGameplayTagEvent(SprintTagHandle, SprintTag, EGameplayTagEventType::NewOrRemoved);
	}

	Super::NativeDestruct();
}

void UPTWCrosshair::OnStateTagChanged(const FGameplayTag Tag, int32 NewCount)
{
	UpdateVisibility();
}

void UPTWCrosshair::UpdateVisibility()
{
	if (!CachedASC) return;

	bool bHasWeapon = CachedASC->HasMatchingGameplayTag(EquipTag);
	bool bIsSprinting = CachedASC->HasMatchingGameplayTag(SprintTag);

	bool bShouldShow = bHasWeapon && !bIsSprinting;

	SetCrosshairVisibility(bShouldShow);
}

void UPTWCrosshair::SetCrosshairVisibility(bool bVisible)
{
	SetVisibility(bVisible ? ESlateVisibility::Visible : ESlateVisibility::Hidden);
}
