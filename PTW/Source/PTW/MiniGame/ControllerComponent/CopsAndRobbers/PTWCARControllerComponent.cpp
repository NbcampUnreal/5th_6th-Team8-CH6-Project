#include "PTWCARControllerComponent.h"
#include "Components/WidgetComponent.h"
#include "CoreFramework/PTWPlayerCharacter.h"
#include "GameFramework/PlayerState.h"

UPTWCARControllerComponent::UPTWCARControllerComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
	SetIsReplicatedByDefault(true);
}

void UPTWCARControllerComponent::ClientRPC_TargetDestroyNameTag_Implementation(APlayerState* TargetState)
{
	if (IsRunningDedicatedServer()) return;
	if (!IsValid(TargetState)) return;
	
	if (APTWPlayerCharacter* TargetCharacter = TargetState->GetPawn<APTWPlayerCharacter>())
	{
		TargetCharacter->GetNameTagWidget()->DestroyComponent();
	}
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
