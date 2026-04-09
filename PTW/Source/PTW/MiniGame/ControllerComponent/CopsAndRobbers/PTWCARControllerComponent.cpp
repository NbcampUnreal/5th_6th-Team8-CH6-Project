// Fill out your copyright notice in the Description page of Project Settings.


#include "PTWCARControllerComponent.h"

#include "Components/WidgetComponent.h"
#include "CoreFramework/PTWPlayerCharacter.h"
#include "CoreFramework/Game/GameState/PTWGameState.h"
#include "GameFramework/PlayerState.h"
#include "Net/UnrealNetwork.h"

UPTWCARControllerComponent::UPTWCARControllerComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
	SetIsReplicatedByDefault(true);
}

void UPTWCARControllerComponent::BeginPlay()
{
	Super::BeginPlay();
}

void UPTWCARControllerComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	Super::EndPlay(EndPlayReason);
}

void UPTWCARControllerComponent::TickComponent(float DeltaTime, ELevelTick TickType,
                                               FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
}

void UPTWCARControllerComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	// DOREPLIFETIME(ThisClass, TeamId);
}

void UPTWCARControllerComponent::InitializeController()
{
}

void UPTWCARControllerComponent::ClientRPC_TargetDestroyNameTag_Implementation(APlayerState* TargetState)
{
	if (IsRunningDedicatedServer()) return;
	if (!IsValid(TargetState)) return;
		
	if (IPTWPlayerDataInterface* TargetRoundData = Cast<IPTWPlayerDataInterface>(TargetState))
	{
		APTWPlayerCharacter* TargetCharacter = TargetState->GetPawn<APTWPlayerCharacter>();
		if (!IsValid(TargetCharacter)) return;
		
		TargetCharacter->GetNameTagWidget()->DestroyComponent();
	}
}
