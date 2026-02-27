// Fill out your copyright notice in the Description page of Project Settings.


#include "System/Prop/PTWPropSubsystem.h"   
#include "EngineUtils.h"
#include "Components/PrimitiveComponent.h"

void UPTWPropSubsystem::RegisterByActorTag(FName GroupTag)
{
	if (GroupTag.IsNone()) return;

	TArray<TWeakObjectPtr<AActor>>& List = GroupToActors.FindOrAdd(GroupTag);
	List.Reset();

	for (TActorIterator<AActor> It(GetWorld()); It; ++It)
	{
		AActor* Actor = *It;
		if (!IsValid(Actor)) continue;

		if (Actor->ActorHasTag(GroupTag))
		{
			List.Add(Actor);
		}
	}
}

void UPTWPropSubsystem::SetGroupEnabled(FName GroupTag, bool bEnabled)
{
	TArray<TWeakObjectPtr<AActor>>* ListPtr = GroupToActors.Find(GroupTag);
	if (!ListPtr) return;

	for (TWeakObjectPtr<AActor>& WeakActor : *ListPtr)
	{
		AActor* Actor = WeakActor.Get();
		if (!IsValid(Actor)) continue;

		ApplyActorEnabled(Actor, bEnabled);
	}
}

void UPTWPropSubsystem::ApplyActorEnabled(AActor* Actor, bool bEnabled)
{
	Actor->SetActorHiddenInGame(!bEnabled);

	TArray<UPrimitiveComponent*> PrimComps;
	Actor->GetComponents<UPrimitiveComponent>(PrimComps);

	for (UPrimitiveComponent* Comp : PrimComps)
	{
		if (!Comp) continue;
		Comp->SetCollisionEnabled(bEnabled ? ECollisionEnabled::QueryAndPhysics : ECollisionEnabled::NoCollision);
	}
}

void UPTWPropSubsystem::ApplySeededRandomByActorTag(FName GroupTag, int32 Seed, float EnableChance)
{
	EnableChance = FMath::Clamp(EnableChance, 0.f, 1.f);

	TArray<AActor*> Actors;
	for (TActorIterator<AActor> It(GetWorld()); It; ++It)
	{
		AActor* Actor = *It;
		if (IsValid(Actor) && Actor->ActorHasTag(GroupTag))
		{
			Actors.Add(Actor);
		}
	}
	
	Actors.Sort([](const AActor& A, const AActor& B)
	{
		return A.GetPathName() < B.GetPathName();
	});

	FRandomStream Stream(Seed);

	for (AActor* Actor : Actors)
	{
		const bool bEnable = (Stream.FRand() < EnableChance);
		ApplyActorEnabled(Actor, bEnable);
	}
}

void UPTWPropSubsystem::ApplyRoundPropSeed(int32 Seed)
{
	ApplySeededRandomByActorTag("Group_A", Seed, 0.5f);
	ApplySeededRandomByActorTag("Group_B", Seed, 0.5f);
	ApplySeededRandomByActorTag("Group_C", Seed, 0.5f);
	ApplySeededRandomByActorTag("Group_D", Seed, 0.5f);

	// 추가 그룹 설정
	// ApplySeededRandomByActorTag("Group_B", Seed + 1, 0.3f);
}
