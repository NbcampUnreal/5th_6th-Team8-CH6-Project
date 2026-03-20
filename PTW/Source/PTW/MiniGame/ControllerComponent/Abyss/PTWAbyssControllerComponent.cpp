// Fill out your copyright notice in the Description page of Project Settings.


#include "MiniGame/ControllerComponent/Abyss/PTWAbyssControllerComponent.h"

#include "Engine/PostProcessVolume.h"
#include "EngineUtils.h"

UPTWAbyssControllerComponent::UPTWAbyssControllerComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

void UPTWAbyssControllerComponent::BeginPlay()
{
	Super::BeginPlay();
}

void UPTWAbyssControllerComponent::SetAbyssDark(bool bEnable)
{
	if (!GetWorld()) return;

	if (!CachedAbyssPP)
	{
		CacheAbyssPP();
	}

	if (!CachedAbyssPP) return;

	CachedAbyssPP->bEnabled = true;
	CachedAbyssPP->BlendWeight = bEnable ? 0.3f : 0.0f;
}

void UPTWAbyssControllerComponent::CacheAbyssPP()
{
	if (!GetWorld()) return;
	if (CachedAbyssPP) return;

	for (TActorIterator<APostProcessVolume> It(GetWorld()); It; ++It)
	{
		APostProcessVolume* PP = *It;
		if (!PP) continue;

		if (PP->ActorHasTag(FName("AbyssPP")))
		{
			CachedAbyssPP = PP;
			break;
		}
	}
}

