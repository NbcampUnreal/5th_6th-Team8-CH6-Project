// Fill out your copyright notice in the Description page of Project Settings.


#include "MiniGame/Component/PTWGameModeBaseComponent.h"

#include "CoreFramework/Game/GameState/PTWGameState.h"


UPTWGameModeBaseComponent::UPTWGameModeBaseComponent()
{
}

void UPTWGameModeBaseComponent::BeginPlay()
{
	Super::BeginPlay();

	GameState = Cast<APTWGameState>(GetWorld()->GetGameState());
}
