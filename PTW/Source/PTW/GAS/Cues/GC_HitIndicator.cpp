// Fill out your copyright notice in the Description page of Project Settings.


#include "GAS/Cues/GC_HitIndicator.h"

#include "CoreFramework/PTWPlayerCharacter.h"
#include "GameFramework/PlayerController.h"
#include "Engine/LocalPlayer.h"

#include "UI/PTWUISubsystem.h"
#include "PTWGameplayTag/GameplayTags.h"

bool UGC_HitIndicator::OnExecute_Implementation(AActor* MyTarget, const FGameplayCueParameters& Parameters) const
{
	APTWPlayerCharacter* PC = Cast<APTWPlayerCharacter>(MyTarget);
	if (!PC) return false;

	// 로컬 플레이어만 실행
	if (!PC->IsLocallyControlled()) return false;

	APlayerController* Controller = PC->GetController<APlayerController>();
	if (!Controller) return false;

	ULocalPlayer* LocalPlayer = Controller->GetLocalPlayer();
	if (!LocalPlayer) return false;

	UPTWUISubsystem* UISubsystem = LocalPlayer->GetSubsystem<UPTWUISubsystem>();
	if (!UISubsystem) return false;

	// 헤드샷 여부
	bool bHeadShot = Parameters.AggregatedSourceTags.HasTag(
		GameplayTags::State::HitReaction_HeadShot
	);

	// UI 호출
	UISubsystem->ShowHitIndicator(HitIndicatorWidgetClass, bHeadShot);

	return true;
}
