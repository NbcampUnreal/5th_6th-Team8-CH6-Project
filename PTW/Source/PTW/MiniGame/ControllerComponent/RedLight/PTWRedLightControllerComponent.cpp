// Fill out your copyright notice in the Description page of Project Settings.


#include "MiniGame/ControllerComponent/RedLight/PTWRedLightControllerComponent.h"
#include "MiniGame/Widget/RedLight/PTWRedLightChargeWidget.h"
#include "GameFramework/PlayerController.h"

void UPTWRedLightControllerComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	if (EndPlayReason != EEndPlayReason::EndPlayInEditor && EndPlayReason != EEndPlayReason::Quit)
	{
		if (IsValid(TaggerUIInstance) && TaggerUIInstance->IsInViewport())
		{
			TaggerUIInstance->RemoveFromParent();
		}
	}

	TaggerUIInstance = nullptr;
	Super::EndPlay(EndPlayReason);
}

void UPTWRedLightControllerComponent::ShowTaggerUI()
{
	APlayerController* PC = Cast<APlayerController>(GetOwner());
	if (!PC || !PC->IsLocalController()) return;

	if (!TaggerUIInstance && TaggerUIClass)
	{
		TaggerUIInstance = CreateWidget<UPTWRedLightChargeWidget>(PC, TaggerUIClass);
	}

	if (TaggerUIInstance && !TaggerUIInstance->IsInViewport())
	{
		TaggerUIInstance->AddToViewport(100);
	}
}

void UPTWRedLightControllerComponent::HideTaggerUI()
{
	if (TaggerUIInstance && TaggerUIInstance->IsInViewport())
	{
		TaggerUIInstance->RemoveFromParent();
	}
}
