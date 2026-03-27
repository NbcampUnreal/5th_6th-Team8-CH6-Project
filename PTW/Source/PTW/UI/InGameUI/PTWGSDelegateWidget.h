// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "CoreFramework/Game/GameState/PTWGameState.h"
#include "PTWGSDelegateWidget.generated.h"

class UPTWInventoryWidget;
class UPTWMiniGameInventory;
class UPTWMiniGameTitle;

/**
 * 
 */
UCLASS()
class PTW_API UPTWGSDelegateWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	// 초기 바인딩 시작점
	void InitializeDelegateWidget();

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UPTWInventoryWidget> InventoryWidget;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UPTWMiniGameInventory> MiniGameInventoryWidget;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UPTWMiniGameTitle> MiniGameTitle;

protected:
	virtual void NativeConstruct() override;
	virtual void NativeDestruct() override;

	// GameState 로딩 대기용
	void TryBindGameState();
	FTimerHandle GameStateBindTimerHandle;

	UFUNCTION()
	void HandleGamePhaseChanged(EPTWGamePhase Phase);

	UFUNCTION()
	void HandleRoulettePhaseChanged(FPTWRouletteData RouletteData);
};
