// Fill out your copyright notice in the Description page of Project Settings.


#include "CoreFramework/Character/Component/PTWDeveloperComponent.h"
#include "System/PTWDeveloperSubsystem.h"

UPTWDeveloperComponent::UPTWDeveloperComponent()
{
	PrimaryComponentTick.bCanEverTick = false;

	SetIsReplicatedByDefault(true);
}

void UPTWDeveloperComponent::Server_ExecuteDevCommand_Implementation(FName CommandName, int32 IntParam, float FloatParam, const FString& StringParam)
{

	if (UPTWDeveloperSubsystem* DevSub = GetWorld()->GetSubsystem<UPTWDeveloperSubsystem>())
	{
		APlayerController* CallerPC = Cast<APlayerController>(GetOwner());
		DevSub->ProcessServerCommand(CallerPC, CommandName, IntParam, FloatParam, StringParam);
	}
}
