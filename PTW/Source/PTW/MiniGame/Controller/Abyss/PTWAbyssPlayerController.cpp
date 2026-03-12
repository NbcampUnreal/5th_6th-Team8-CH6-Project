// Fill out your copyright notice in the Description page of Project Settings.


#include "MiniGame/Controller/Abyss/PTWAbyssPlayerController.h"

#include "EngineUtils.h"
#include "Engine/PostProcessVolume.h"

void APTWAbyssPlayerController::Client_SetAbyssDark_Implementation(bool bEnable)
{
	if (!GetWorld()) return;

	if (!CachedAbyssPP)
	{
		CacheAbyssPP();
	}

	if (!CachedAbyssPP) return;

	CachedAbyssPP->bEnabled = true;
	CachedAbyssPP->BlendWeight = bEnable ? 1.0f : 0.0f;
}

void APTWAbyssPlayerController::CacheAbyssPP()
{
	if (!GetWorld()) return;
	if (CachedAbyssPP) return;

	for (TActorIterator<APostProcessVolume> It(GetWorld()); It; ++It)
	{
		if (It->ActorHasTag(FName("AbyssPP")))
		{
			CachedAbyssPP = *It;
			break;
		}
	}
}

