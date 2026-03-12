// Fill out your copyright notice in the Description page of Project Settings.


#include "MiniGame/ControllerComponent/GhostChase/PTWGhostChaseControllerComponent.h"
#include "Components/WidgetComponent.h"
#include "Blueprint/UserWidget.h"
#include "Net/UnrealNetwork.h"
#include "UI/CharacterUI/PTWPlayerName.h"

UPTWGhostChaseControllerComponent::UPTWGhostChaseControllerComponent()
{
	SetIsReplicatedByDefault(true);
}

void UPTWGhostChaseControllerComponent::SetTarget(APawn* NewTarget)
{
	CurrentTargetPawn = NewTarget;
}

bool UPTWGhostChaseControllerComponent::IsTarget(APawn* Pawn) const
{
	return Pawn && Pawn == CurrentTargetPawn;
}

void UPTWGhostChaseControllerComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME_CONDITION(UPTWGhostChaseControllerComponent, CurrentTargetPawn, COND_OwnerOnly);
}

void UPTWGhostChaseControllerComponent::ApplyNameTagHighlight(APawn* TargetPawn, UWidgetComponent* WidgetComp)
{
	if (!WidgetComp) return;

	// 위젯 클래스 캐스팅
	UPTWPlayerName* NameTagWidget = Cast<UPTWPlayerName>(WidgetComp->GetUserWidgetObject());
	if (!NameTagWidget) return;

	// 타겟 여부에 따라 색상 지정
	if (IsTarget(TargetPawn))
	{
		NameTagWidget->SetNameColor(TargetHighlightColor);
	}
	else
	{
		NameTagWidget->SetNameColor(FLinearColor::White); // 기본 색상
	}
}
