// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "PTWLobbyItemRow.generated.h"

class UPTWLobbyItemDefinition;

USTRUCT(BlueprintType)
struct FPTWLobbyItemRow : public FTableRowBase
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere)
	TSoftObjectPtr<UPTWLobbyItemDefinition> LobbyItemDefinition;
};
