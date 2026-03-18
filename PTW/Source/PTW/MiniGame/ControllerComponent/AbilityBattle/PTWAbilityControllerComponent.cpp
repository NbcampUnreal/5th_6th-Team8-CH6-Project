// Fill out your copyright notice in the Description page of Project Settings.


#include "MiniGame/ControllerComponent/AbilityBattle/PTWAbilityControllerComponent.h"

#include "Blueprint/UserWidget.h"
#include "CoreFramework/PTWPlayerController.h"
#include "CoreFramework/PTWPlayerState.h"
#include "Kismet/GameplayStatics.h"
#include "MiniGame/Data/AbilityBattle/PTWAbilityDefinition.h"
#include "MiniGame/Data/AbilityBattle/PTWAbilityRow.h"
#include "UI/MiniGame/AbilityBattle/PTWAbilityDraftWidget.h"


UPTWAbilityControllerComponent::UPTWAbilityControllerComponent()
{
	
	PrimaryComponentTick.bCanEverTick = false;
	SetIsReplicatedByDefault(true);
}

void UPTWAbilityControllerComponent::SetGameInputMode()
{
	APTWPlayerController* PlayerController = Cast<APTWPlayerController>(GetOwner());
	if (!PlayerController) return;

	FInputModeGameOnly InputModeGameOnly;
	PlayerController->SetInputMode(InputModeGameOnly);
	PlayerController->bShowMouseCursor = false;
}

void UPTWAbilityControllerComponent::Server_SelectedAbility_Implementation(FName RowId)
{
	if (!AbilityDataTable) return;

	APTWPlayerController* PlayerController = Cast<APTWPlayerController>(GetOwner());
	if (!PlayerController) return;
	APTWPlayerState* PlayerState = PlayerController->GetPlayerState<APTWPlayerState>();
	if (!PlayerState) return;

	FPTWAbilityRow* Row = AbilityDataTable->FindRow<FPTWAbilityRow>(RowId, TEXT(""));
	if (!Row) return;

	UPTWAbilityDefinition* Definition = Row->AbilityDefinition.LoadSynchronous();
	if (!Definition) return;
	
	PlayerState->InjectEffect(Definition->EffectClass);

	UE_LOG(LogTemp, Log, TEXT("Server_SelectedAbility_Implementation"));
}

void UPTWAbilityControllerComponent::Client_ShowDraftUI_Implementation(const TArray<FName>& RowId)
{
	if (!DraftWidgetClass || DraftWidget) return;
	
	APlayerController* PlayerController = Cast<APlayerController>(GetOwner()); 
	if (!PlayerController || !PlayerController->IsLocalController()) return;
	
	DraftWidget = CreateWidget<UPTWAbilityDraftWidget>(PlayerController,DraftWidgetClass);
	if (!DraftWidget) return;
	
	DraftWidget->GenerateAbilityBoxes(RowId);
	DraftWidget->AddToViewport(50);
	
	GetWorld()->GetTimerManager().SetTimerForNextTick([this,PlayerController]()
	{
		if (DraftWidget && PlayerController)
		{
			FInputModeUIOnly InputModeUIOnly;
			InputModeUIOnly.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);
			PlayerController->SetInputMode(InputModeUIOnly);

			// FInputModeGameAndUI InputModeGameAndUI;
			// InputModeGameAndUI.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);
			// PlayerController->SetInputMode(InputModeGameAndUI);
			
			PlayerController->bShowMouseCursor = true;
			
		}
	});
	
	UE_LOG(LogTemp, Log, TEXT("ShowDraftUI"));
}



