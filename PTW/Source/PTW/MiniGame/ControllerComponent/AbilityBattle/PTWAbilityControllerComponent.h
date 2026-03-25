// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "MiniGame/ControllerComponent/PTWBaseControllerComponent.h"
#include "PTWAbilityControllerComponent.generated.h"


class UGameplayEffect;
class UPTWAbilityDraftWidget;

UCLASS(Blueprintable, ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class PTW_API UPTWAbilityControllerComponent : public UPTWBaseControllerComponent
{
	GENERATED_BODY()

public:	
	UPTWAbilityControllerComponent();

	UFUNCTION(Client, Reliable)
	void Client_RespawnPlayer(bool bCanRespawn, int32 RespawnDelay);

	UFUNCTION(Server, Reliable)
	void Server_CallHandleRespawn();

	UFUNCTION(Client, Reliable)
	void Client_ShowDraftUI(const TArray<FName>& RowId);

	UFUNCTION(Client, Reliable)
	void Client_HideDraftUI();

	
	UFUNCTION(Server, Reliable)
	void Server_SelectedAbility(FName RowId);

	UFUNCTION(Client, Reliable)
	void Client_UIInputMode();

	UFUNCTION(Client, Reliable)
	void Client_GameInputMode();
	
	void SetGameInputMode();
	void SetUIInputMode(APlayerController* InPlayerController = nullptr);

	UPROPERTY()
	TObjectPtr<UPTWAbilityDraftWidget> DraftWidget;
protected:
	UPROPERTY(EditDefaultsOnly)
	TSubclassOf<UUserWidget> DraftWidgetClass;
	
	UPROPERTY(EditDefaultsOnly)
	TObjectPtr<UDataTable> AbilityDataTable;
};
