// Fill out your copyright notice in the Description page of Project Settings.


#include "PTWHUD.h"
#include "PTWInGameHUD.h"
#include "AbilitySystemInterface.h" // ASC
#include "AbilitySystemComponent.h" // ASC
#include "CoreFramework/PTWPlayerController.h"
#include "CoreFramework/PTWPlayerCharacter.h"
#include "UI/PTWUISubsystem.h"
#include "UI/InGameUI/PTWDamageIndicator.h"

void APTWHUD::BeginPlay()
{
	Super::BeginPlay();

	FindPlayerController();
}

void APTWHUD::FindPlayerController()
{
	APlayerController* PC = GetOwningPlayerController();
	if (!PC)
	{
		GetWorld()->GetTimerManager().SetTimer(
			FindPlayerControllerTimerHandle,
			this,
			&APTWHUD::FindPlayerController,
			0.1f, // 0.1초 간격으로 체크
			false
		);
	}

	GetWorld()->GetTimerManager().ClearTimer(FindPlayerControllerTimerHandle);

	if (APTWPlayerController* PTWPC = Cast<APTWPlayerController>(PC))
	{
		PTWPC->CreateUI();
	}
}

