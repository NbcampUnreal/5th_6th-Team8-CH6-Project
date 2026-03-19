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
	UE_LOG(LogTemp, Warning, TEXT("UPTWCARControllerComponent::BeginPlay()"));
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
	DOREPLIFETIME(ThisClass, TeamId);
}

void UPTWCARControllerComponent::InitializeController()
{
}

void UPTWCARControllerComponent::DestroyOtherTeamNameTag()
{
	if (IsRunningDedicatedServer()) return;
	
	if (TeamId == -1) return;
	
	UWorld* World = GetWorld();
	if (!IsValid(World)) return;
	
	APTWGameState* GS = World->GetGameState<APTWGameState>();
	if (!IsValid(GS)) return;
	
	for (APlayerState* TargetPS : GS->PlayerArray)
	{
		if (!IsValid(TargetPS)) continue;
		
		if (IPTWPlayerRoundDataInterface* TargetRoundData = Cast<IPTWPlayerRoundDataInterface>(TargetPS))
		{
			int32 TargetTeamId = TargetRoundData->GetTeamId();
			if (TargetTeamId == -1) continue;

			if (TeamId != TargetTeamId)
			{
				APTWPlayerCharacter* TargetCharacter = TargetPS->GetPawn<APTWPlayerCharacter>();
				if (!IsValid(TargetCharacter)) continue;
		
				TargetCharacter->GetNameTagWidget()->DestroyComponent();
			}
		}
	}
}

void UPTWCARControllerComponent::SetTeamId(int32 NewTeamId)
{
	TeamId = NewTeamId;
	if (GetNetMode() != NM_DedicatedServer)
	{
		OnRep_TeamId();
	}
}

void UPTWCARControllerComponent::OnRep_TeamId()
{
	DestroyOtherTeamNameTag();
}
