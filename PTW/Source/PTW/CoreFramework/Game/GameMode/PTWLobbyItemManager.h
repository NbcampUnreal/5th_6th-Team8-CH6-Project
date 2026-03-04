// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Object.h"
#include "PTWLobbyItemManager.generated.h"

class APTWPlayerState;
/**
 * 
 */
UCLASS()
class PTW_API UPTWLobbyItemManager : public UObject
{
	GENERATED_BODY()

public:
	void ApplyLobbyItem(APTWPlayerState* Buyer, const FName ItemId, APTWPlayerState* WinTarget = nullptr);

	void InitLobbyItemTable(UDataTable* DataTable);
private:
	
	UPROPERTY()
	TObjectPtr<UDataTable> LobbyItemTable;
	
};
