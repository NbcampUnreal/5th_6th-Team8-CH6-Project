// Fill out your copyright notice in the Description page of Project Settings.


#include "PTWGA_RBAoEStop.h"

#include "PTW.h"
#include "CoreFramework/PTWPlayerCharacter.h"
#include "CoreFramework/PTWPlayerController.h"
#include "Engine/OverlapResult.h"

void UPTWGA_RBAoEStop::ActivateAbility(const FGameplayAbilitySpecHandle Handle,
                                       const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo,
                                       const FGameplayEventData* TriggerEventData)
{
	Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);
	
	if (!CommitAbility(Handle, ActorInfo, ActivationInfo))
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, false);
		return;
	}
	
	//if (!HasAuthority(&CurrentActivationInfo)) return;
	
	APTWPlayerCharacter* MyPC = Cast<APTWPlayerCharacter>(GetAvatarActorFromActorInfo());
	if (!MyPC) return;
	
	TArray<FOverlapResult> OverlapResults;
	
	FVector ExplosionLocation = MyPC->GetActorLocation();
	FCollisionShape SphereShape = FCollisionShape::MakeSphere(DetectRad);
	FCollisionQueryParams CollisionParams;
				
	CollisionParams.AddIgnoredActor(MyPC);
	
	GetWorld()->OverlapMultiByChannel(
		OverlapResults,
		ExplosionLocation,
		FQuat::Identity,
		ECC_WeaponAttack,
		SphereShape,
		CollisionParams
		);
	
	ApplyAoEEffect(OverlapResults);
	EndAbility(Handle, CurrentActorInfo, CurrentActivationInfo, false, false);
}

void UPTWGA_RBAoEStop::ApplyAoEEffect(TArray<FOverlapResult>& OverlapResults)
{
	for (const FOverlapResult& Result : OverlapResults)
	{
		IPTWCombatInterface* HitActor = Cast<IPTWCombatInterface>(Result.GetActor());
		if (!HitActor) continue;
		HitActor->ApplyGameplayEffectToSelf(StunEffect, 1.0f, FGameplayEffectContextHandle());
		
		APTWPlayerCharacter* HitActorCH = Cast<APTWPlayerCharacter>(HitActor);
		if (!HitActorCH) continue;
		APTWPlayerController* PC = Cast<APTWPlayerController>(HitActorCH->GetController());
		if (!PC) continue;
		
		SendActivateMsg(PC);
	}
}

void UPTWGA_RBAoEStop::SendActivateMsg(APTWPlayerController* TargetController)
{
	APTWPlayerCharacter* MyCharacter =  Cast<APTWPlayerCharacter>(GetAvatarActorFromActorInfo());
	if (!MyCharacter) return;
	
	APTWPlayerController* MyController = Cast<APTWPlayerController>(MyCharacter->GetController());
	if (!MyController) return;
	
	
#define LOCTEXT_NAMESPACE "DeliveryGameMode"
	FText DeliveryItemStunMessage = LOCTEXT("GetStun!", "Stunned by an enemy item!");
#undef LOCTEXT_NAMESPACE
	TargetController->SendMessage(DeliveryItemStunMessage, ENotificationPriority::Normal, 3);
}
