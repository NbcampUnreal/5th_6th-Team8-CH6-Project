// Fill out your copyright notice in the Description page of Project Settings.


#include "GAS/Abilities/PTWGA_BombExplode.h"

#include "Engine/World.h"
#include "Engine/OverlapResult.h" 
#include "CollisionQueryParams.h"
#include "CollisionShape.h"
#include "GameFramework/Pawn.h"
#include "GameplayTagContainer.h"
#include "PTWGameplayTag/GameplayTags.h"

UPTWGA_BombExplode::UPTWGA_BombExplode()
{
	NetExecutionPolicy = EGameplayAbilityNetExecutionPolicy::ServerOnly;
	InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;

	FAbilityTriggerData TriggerData;
	TriggerData.TriggerTag = GameplayTags::Event::Bomb_Explode;
	TriggerData.TriggerSource = EGameplayAbilityTriggerSource::GameplayEvent;
	AbilityTriggers.Add(TriggerData);
}

void UPTWGA_BombExplode::ActivateAbility(
	const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo,
	const FGameplayAbilityActivationInfo ActivationInfo,
	const FGameplayEventData* TriggerEventData)
{
	if (!CommitAbility(Handle, ActorInfo, ActivationInfo))
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}

	AActor* BombActor = ActorInfo ? ActorInfo->AvatarActor.Get() : nullptr;
	if (!BombActor)
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}

	UWorld* World = BombActor->GetWorld();
	if (!World)
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}

	const FVector Center = BombActor->GetActorLocation();

	TArray<FOverlapResult> Overlaps;
	FCollisionQueryParams Params(SCENE_QUERY_STAT(BombExplode), false, BombActor);

	const bool bHit = World->OverlapMultiByObjectType(
		Overlaps,
		Center,
		FQuat::Identity,
		FCollisionObjectQueryParams(FCollisionObjectQueryParams::InitType::AllDynamicObjects),
		FCollisionShape::MakeSphere(ExplosionRadius),
		Params
	);

	if (bHit)
	{
		for (const FOverlapResult& R : Overlaps)
		{
			APawn* Pawn = Cast<APawn>(R.GetActor());
			if (!Pawn) continue;

			UE_LOG(LogTemp, Warning, TEXT("[BombExplode] %s hit"), *Pawn->GetName());
		}
	}

	EndAbility(Handle, ActorInfo, ActivationInfo, false, false);
}


