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

static int32 MixSeed(int32 BaseSeed, FName GroupTag)
{
	return HashCombineFast(BaseSeed, GetTypeHash(GroupTag));
}

void UPTWPropSubsystem::ApplyRoundPropSeed(int32 Seed)
{
	ApplySeededRandomGroupEnabled("Group_A", MixSeed(Seed, "Group_A"), 0.5f);
	ApplySeededRandomGroupEnabled("Group_B", MixSeed(Seed, "Group_B"), 0.5f);
	ApplySeededRandomGroupEnabled("Group_C", MixSeed(Seed, "Group_C"), 0.5f);
	ApplySeededRandomGroupEnabled("Group_D", MixSeed(Seed, "Group_D"), 0.5f);

}

void UPTWPropSubsystem::ApplySeededRandomGroupEnabled(FName GroupTag, int32 Seed, float EnableChance)
{
	EnableChance = FMath::Clamp(EnableChance, 0.f, 1.f);
	
	FRandomStream Stream(Seed);
	const bool bEnableGroup = (Stream.FRand() < EnableChance);
	
	RegisterByActorTag(GroupTag);
	SetGroupEnabled(GroupTag, bEnableGroup);
}

