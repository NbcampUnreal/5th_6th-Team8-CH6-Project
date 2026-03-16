// Fill out your copyright notice in the Description page of Project Settings.


#include "MiniGame/ControllerComponent/AbilityBattle/PTWAbilityControllerComponent.h"

#include "Blueprint/UserWidget.h"
#include "Kismet/GameplayStatics.h"
#include "UI/MiniGame/AbilityBattle/PTWAbilityDraftWidget.h"


UPTWAbilityControllerComponent::UPTWAbilityControllerComponent()
{
	
	PrimaryComponentTick.bCanEverTick = false;


}

void UPTWAbilityControllerComponent::Client_ShowDraftUI_Implementation()
{
	if (!DraftWidgetClass || DraftWidget) return;

	APlayerController* PlayerController = Cast<APlayerController>(GetOwner()); 
	if (!PlayerController || !PlayerController->IsLocalController()) return;
	
	DraftWidget = CreateWidget<UPTWAbilityDraftWidget>(PlayerController,DraftWidgetClass);
	if (!DraftWidget) return;
	
	DraftWidget->GenerateAbilityBoxes(5);
	DraftWidget->AddToViewport();
	
	GetWorld()->GetTimerManager().SetTimerForNextTick([this,PlayerController]()
	{
		if (DraftWidget && PlayerController)
		{
			FInputModeUIOnly InputModeUIOnly;
			InputModeUIOnly.SetWidgetToFocus(DraftWidget->TakeWidget());
			InputModeUIOnly.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);
	
			PlayerController->SetInputMode(InputModeUIOnly);
			PlayerController->bShowMouseCursor = true;
		}
	});
	
	UE_LOG(LogTemp, Log, TEXT("ShowDraftUI"));
}





