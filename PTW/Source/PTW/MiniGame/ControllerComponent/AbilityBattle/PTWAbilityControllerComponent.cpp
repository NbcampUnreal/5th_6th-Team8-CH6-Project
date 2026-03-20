// Fill out your copyright notice in the Description page of Project Settings.


#include "MiniGame/ControllerComponent/AbilityBattle/PTWAbilityControllerComponent.h"

#include "Blueprint/UserWidget.h"
#include "CoreFramework/PTWPlayerController.h"
#include "CoreFramework/PTWPlayerState.h"
#include "Kismet/GameplayStatics.h"
#include "MiniGame/Data/AbilityBattle/PTWAbilityDefinition.h"
#include "MiniGame/Data/AbilityBattle/PTWAbilityRow.h"
#include "MiniGame/GameMode/PTWAbilityBattleGameMode.h"
#include "MiniGame/PlayerStateComponent/PTWAbilityBattlePSComponent.h"
#include "UI/MiniGame/AbilityBattle/PTWAbilityDraftWidget.h"


UPTWAbilityControllerComponent::UPTWAbilityControllerComponent()
{
	
	PrimaryComponentTick.bCanEverTick = false;
	SetIsReplicatedByDefault(true);
}

void UPTWAbilityControllerComponent::Client_HideDraftUI_Implementation()
{
	if (DraftWidget)
	{
		DraftWidget->SetVisibility(ESlateVisibility::Collapsed);
	}
}

void UPTWAbilityControllerComponent::SetGameInputMode()
{
	APTWPlayerController* PlayerController = Cast<APTWPlayerController>(GetOwner());
	if (!PlayerController) return;

	FInputModeGameOnly InputModeGameOnly;
	PlayerController->SetInputMode(InputModeGameOnly);
	PlayerController->bShowMouseCursor = false;
}

void UPTWAbilityControllerComponent::Client_GameInputMode_Implementation()
{
	SetGameInputMode();
}


void UPTWAbilityControllerComponent::SetUIInputMode(APlayerController* InPlayerController)
{
	if (!InPlayerController)
	{
		InPlayerController = Cast<APTWPlayerController>(GetOwner());
	}
	if (!InPlayerController) return;
	
	FInputModeUIOnly InputModeUIOnly;
	InputModeUIOnly.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);
	InPlayerController->SetInputMode(InputModeUIOnly);
	InPlayerController->bShowMouseCursor = true;
	InPlayerController->FlushPressedKeys();
}

void UPTWAbilityControllerComponent::Server_SelectedAbility_Implementation(FName RowId)
{
	if (!AbilityDataTable) return;

	APTWPlayerController* PlayerController = Cast<APTWPlayerController>(GetOwner());
	if (!PlayerController) return;
	APTWPlayerState* PlayerState = PlayerController->GetPlayerState<APTWPlayerState>();
	if (!PlayerState) return;

	UPTWAbilityBattlePSComponent* PlayerStateComponent = Cast<UPTWAbilityBattlePSComponent>(PlayerState->GetMiniGameComponent());
	if (!PlayerStateComponent) return;
	
	FPTWAbilityRow* Row = AbilityDataTable->FindRow<FPTWAbilityRow>(RowId, TEXT(""));
	if (!Row) return;

	UPTWAbilityDefinition* Definition = Row->AbilityDefinition.LoadSynchronous();
	if (!Definition) return;
	
	PlayerState->InjectEffect(Definition->EffectClass);
	
	PlayerStateComponent->DecreaseDraftCharges();
	// 여기서 게임모드 스폰 함수 호출 하면 될듯

	APTWAbilityBattleGameMode* AbilityBattleGameMode = Cast<APTWAbilityBattleGameMode>(GetWorld()->GetAuthGameMode());
	if (!AbilityBattleGameMode) return;

	if (PlayerStateComponent->bFirstDraftCompleted)
	{
		AbilityBattleGameMode->HandleRespawn(PlayerController);
	}
	
	PlayerStateComponent->bFirstDraftCompleted = true;
	
	UE_LOG(LogTemp, Log, TEXT("Server_SelectedAbility_Implementation"));
}

void UPTWAbilityControllerComponent::Client_ShowDraftUI_Implementation(const TArray<FName>& RowId)
{
	if (!DraftWidgetClass) return;
	
	APlayerController* PlayerController = Cast<APlayerController>(GetOwner()); 
	if (!PlayerController || !PlayerController->IsLocalController()) return;

	if (!DraftWidget)
	{
		DraftWidget = CreateWidget<UPTWAbilityDraftWidget>(PlayerController,DraftWidgetClass);
		DraftWidget->AddToViewport(50);
	}

	DraftWidget->GenerateAbilityBoxes(RowId);
	DraftWidget->SetVisibility(ESlateVisibility::Visible);

	GetWorld()->GetTimerManager().SetTimerForNextTick([this,PlayerController]()
	{
		if (DraftWidget && PlayerController)
		{
			SetUIInputMode(PlayerController);
		}
	});
	
	UE_LOG(LogTemp, Log, TEXT("ShowDraftUI"));
}



