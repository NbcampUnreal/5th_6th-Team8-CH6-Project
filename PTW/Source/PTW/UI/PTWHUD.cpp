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

	/* 위젯 클래스가 에디터에서 설정되었는지 확인 */
	if (!InGameHUDClass)
	{
		return;
	}

	/* 위젯 인스턴스 생성 */
	if (!InGameHUDInstance)
	{
		InGameHUDInstance = CreateWidget<UPTWInGameHUD>(GetWorld(), InGameHUDClass);
		if (InGameHUDInstance)
		{
			InGameHUDInstance->AddToViewport();
		}
	}
	
	/* 위젯 생성 후 초기화 요청 */
	if (APTWPlayerController* PC =
		Cast<APTWPlayerController>(GetOwningPlayerController()))
	{
		PC->TryInitializeHUD();
	}
}

void APTWHUD::InitializeHUD(UAbilitySystemComponent* ASC)
{
	if (bASCInitialized) return;

	if (InGameHUDInstance && ASC)
	{
		bASCInitialized = true;
		InGameHUDInstance->InitializeUI(ASC);
	}

	if (DamageIndicatorClass)
	{
		if (ULocalPlayer* LP = GetOwningPlayerController()->GetLocalPlayer())
		{
			if (UPTWUISubsystem* UISubsystem = LP->GetSubsystem<UPTWUISubsystem>())
			{
				UISubsystem->SetDamageIndicatorClass(DamageIndicatorClass);
			}
		}
	}
}

void APTWHUD::SetCrosshairVisibility(bool bVisible)
{
	if (InGameHUDInstance)
	{
		InGameHUDInstance->SetCrosshairVisibility(bVisible);
	}
}
