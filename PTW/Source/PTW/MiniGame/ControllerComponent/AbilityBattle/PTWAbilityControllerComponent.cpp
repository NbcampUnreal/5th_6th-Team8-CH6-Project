// Fill out your copyright notice in the Description page of Project Settings.


#include "MiniGame/ControllerComponent/AbilityBattle/PTWAbilityControllerComponent.h"

#include "AbilitySystemComponent.h"
#include "GameplayEffectTypes.h"
#include "Blueprint/UserWidget.h"
#include "Components/HorizontalBox.h"
#include "CoreFramework/PTWPlayerController.h"
#include "CoreFramework/PTWPlayerState.h"
#include "Debug/PTWLogCategorys.h"
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
		DraftWidget->HorizontalBox->SetVisibility(ESlateVisibility::Collapsed);
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


void UPTWAbilityControllerComponent::Client_UIInputMode_Implementation()
{
	SetUIInputMode();
}

void UPTWAbilityControllerComponent::SetUIInputMode(APlayerController* InPlayerController)
{
	UE_LOG(Log_AbilityControllerComponent, Log, TEXT("SetUIInputMode 진입"));

	if (!InPlayerController)
	{
		UE_LOG(Log_AbilityControllerComponent, Warning, TEXT("InPlayerController nullptr, Owner에서 캐스팅 시도"));

		InPlayerController = Cast<APTWPlayerController>(GetOwner());

		if (InPlayerController)
		{
			UE_LOG(Log_AbilityControllerComponent, Log, TEXT("Owner -> PlayerController 캐스팅 성공: %s"), *InPlayerController->GetName());
		}
		else
		{
			UE_LOG(Log_AbilityControllerComponent, Error, TEXT("Owner 캐스팅 실패"));
		}
	}
	else
	{
		UE_LOG(Log_AbilityControllerComponent, Log, TEXT("InPlayerController 유효: %s"), *InPlayerController->GetName());
	}

	if (!InPlayerController)
	{
		UE_LOG(Log_AbilityControllerComponent, Error, TEXT("PlayerController 없음, 함수 종료"));
		return;
	}

	UE_LOG(Log_AbilityControllerComponent, Log, TEXT("UI InputMode 설정 시작"));

	FInputModeUIOnly InputModeUIOnly;
	InputModeUIOnly.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);

	InPlayerController->SetInputMode(InputModeUIOnly);
	InPlayerController->bShowMouseCursor = true;
	InPlayerController->FlushPressedKeys();

	UE_LOG(Log_AbilityControllerComponent, Log, TEXT("UI InputMode 설정 완료"));
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

	UAbilitySystemComponent* ASC = PlayerState->GetAbilitySystemComponent();

	FGameplayEffectContextHandle Context = ASC->MakeEffectContext();
	Context.AddSourceObject(ASC->GetOwnerActor());

	FGameplayEffectSpecHandle SpecHandle = ASC->MakeOutgoingSpec(Definition->EffectClass, 1, Context);
	if (!SpecHandle.IsValid()) return;

	ASC->ApplyGameplayEffectSpecToSelf(*SpecHandle.Data.Get());
	
	PlayerStateComponent->DecreaseDraftCharges();

	APTWAbilityBattleGameMode* AbilityBattleGameMode = Cast<APTWAbilityBattleGameMode>(GetWorld()->GetAuthGameMode());
	if (!AbilityBattleGameMode) return;

	if (PlayerStateComponent->bFirstDraftCompleted)
	{
		AbilityBattleGameMode->HandleRespawn(PlayerController);
		//SetGameInputMode();
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

	DraftWidget->bIsSelected = false;
	
	DraftWidget->GenerateAbilityBoxes(RowId);
	DraftWidget->HorizontalBox->SetVisibility(ESlateVisibility::Visible);

	GetWorld()->GetTimerManager().SetTimerForNextTick([this, PlayerController]()
	{
		UE_LOG(Log_AbilityControllerComponent, Log, TEXT("NextTick 진입"));

		if (!this)
		{
			UE_LOG(Log_AbilityControllerComponent, Error, TEXT("this is nullptr"));
			return;
		}

		if (!PlayerController)
		{
			UE_LOG(Log_AbilityControllerComponent, Warning, TEXT("PlayerController is nullptr"));
		}

		if (!DraftWidget)
		{
			UE_LOG(Log_AbilityControllerComponent, Warning, TEXT("DraftWidget is nullptr"));
		}

		if (DraftWidget && PlayerController)
		{
			UE_LOG(Log_AbilityControllerComponent, Log, TEXT("SetUIInputMode 호출"));

			SetUIInputMode(PlayerController);
		}
	});
	
	UE_LOG(LogTemp, Log, TEXT("ShowDraftUI"));
}



