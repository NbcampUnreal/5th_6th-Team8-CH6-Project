// Fill out your copyright notice in the Description page of Project Settings.


#include "PTWHUD.h"
#include "PTWInGameHUD.h"
#include "AbilitySystemInterface.h" // ASC
#include "AbilitySystemComponent.h" // ASC
#include "CoreFramework/PTWPlayerController.h"
#include "CoreFramework/PTWPlayerCharacter.h"

void APTWHUD::BeginPlay()
{
	Super::BeginPlay();

	/* 위젯 클래스가 에디터에서 설정되었는지 확인 */
	if (!InGameHUDClass)
	{
		UE_LOG(LogTemp, Error, TEXT("HUD: !InGameHUDClass"));
		return;
	}

	/* 위젯 인스턴스 미리 생성 */
	if (!InGameHUDInstance)
	{
		InGameHUDInstance = CreateWidget<UPTWInGameHUD>(GetWorld(), InGameHUDClass);
		if (InGameHUDInstance)
		{
			InGameHUDInstance->AddToViewport();
		}
	}
}

void APTWHUD::InitializeHUD(UAbilitySystemComponent* ASC)
{
	if (InGameHUDInstance && ASC)
	{
		InGameHUDInstance->InitializeUI(ASC);
	}
}
