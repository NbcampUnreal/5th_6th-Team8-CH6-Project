// Fill out your copyright notice in the Description page of Project Settings.


#include "MiniGame/Component/PTWSpawnComponent.h"

#include "CoreFramework/PTWPlayerController.h"
#include "CoreFramework/PTWPlayerState.h"
#include "CoreFramework/Interface/PTWGameModeInterface.h"
#include "MiniGame/PTWMiniGameRule.h"

// Sets default values for this component's properties
UPTWSpawnComponent::UPTWSpawnComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

void UPTWSpawnComponent::InitSpawnComponent(const FPTWMiniGameRule* InMiniGameRule)
{
	MiniGameRule = InMiniGameRule;
}

void UPTWSpawnComponent::RespawnPlayer(APTWPlayerController* SpawnPlayerController)
{
	if (MiniGameRule->SpawnRule.bUseRespawn == false) return;
	
	if (IsValid(SpawnPlayerController))
	{
		 GetWorld()->GetTimerManager().ClearTimer(SpawnPlayerController->RespawnTimerHandle);
		TWeakObjectPtr<ThisClass> WeakThis = this;
		TWeakObjectPtr<APTWPlayerController> WeakDeadController = SpawnPlayerController;
		GetWorld()->GetTimerManager().SetTimer(WeakDeadController->RespawnTimerHandle, [WeakThis, WeakDeadController, this]()
		{
			WeakThis->HandleRespawn(WeakDeadController.Get());
			
		}, MiniGameRule->SpawnRule.RespawnDelay, false);
	}
}

void UPTWSpawnComponent::HandleRespawn(APTWPlayerController* PlayerController)
{
	if (!IsValid(PlayerController)) return;

	IPTWGameModeInterface* GameModeInterface = Cast<IPTWGameModeInterface>(GetOwner());
	if (!GameModeInterface) return;
	
	GameModeInterface->RestartPlayer(PlayerController);

	if (MiniGameRule->SpawnRule.bUseSpawnProtection)
	{
		APTWPlayerState* PlayerState = PlayerController->GetPlayerState<APTWPlayerState>();
		if (!PlayerState) return;

		PlayerState->ApplyInvincible(MiniGameRule->SpawnRule.SpawnProtectionTime);
	}
}


