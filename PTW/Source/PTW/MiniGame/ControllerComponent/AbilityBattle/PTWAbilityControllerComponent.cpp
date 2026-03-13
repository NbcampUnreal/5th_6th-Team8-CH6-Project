// Fill out your copyright notice in the Description page of Project Settings.


#include "MiniGame/ControllerComponent/AbilityBattle/PTWAbilityControllerComponent.h"

#include "Blueprint/UserWidget.h"
#include "Kismet/GameplayStatics.h"


UPTWAbilityControllerComponent::UPTWAbilityControllerComponent()
{
	
	PrimaryComponentTick.bCanEverTick = false;


}

void UPTWAbilityControllerComponent::ShowDraftUI()
{
	if (!DraftWidgetClass || DraftWidget) return;

	APlayerController* PlayerController = Cast<APlayerController>(GetOwner()); 
	
	DraftWidget = CreateWidget<UUserWidget>(PlayerController,DraftWidgetClass);
	if (!DraftWidget) return;

	DraftWidget->AddToViewport();
}




