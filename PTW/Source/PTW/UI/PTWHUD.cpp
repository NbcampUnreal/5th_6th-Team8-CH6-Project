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
	if (APTWPlayerController* PC =
		Cast<APTWPlayerController>(GetOwningPlayerController()))
	{
		PC->TryInitializeHUD();
	}

	/* KillLog 델리게이트 바인드 */
	if (APTWPlayerController* PC =
		Cast<APTWPlayerController>(GetOwningPlayerController()))
	{
		PC->OnKillLog.AddUObject(InGameHUDInstance, &UPTWInGameHUD::AddKillLog);
	}
	// <서버에서 킬판정 시>
	// (각 컨트롤러에게 killer, victim 닉네임 전달해주어야 함)
	// PC->OnKillLog.Broadcast(Killer, Victim);

	// + 인자에 무기 종류 추가예정
}

void APTWHUD::InitializeHUD(UAbilitySystemComponent* ASC)
{
	if (bASCInitialized) return;
	UE_LOG(LogTemp, Error, TEXT("HUD get ASC"));

	if (InGameHUDInstance && ASC)
	{
		UE_LOG(LogTemp, Error, TEXT("HUD InitializeHUD"));
		bASCInitialized = true;
		InGameHUDInstance->InitializeUI(ASC);
	}
}

void APTWHUD::SetCrosshairVisibility(bool bVisible)
{
	if (InGameHUDInstance)
	{
		InGameHUDInstance->SetCrosshairVisibility(bVisible);
	}
}
