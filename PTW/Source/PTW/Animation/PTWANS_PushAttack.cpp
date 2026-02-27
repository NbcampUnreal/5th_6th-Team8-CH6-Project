// Fill out your copyright notice in the Description page of Project Settings.


#include "PTWANS_PushAttack.h"

#include "AbilitySystemBlueprintLibrary.h"
#include "PTW.h"
#include "Abilities/GameplayAbilityTypes.h"
#include "Components/SphereComponent.h"
#include "CoreFramework/PTWPlayerCharacter.h"
#include "PTWGameplayTag/GameplayTags.h"

void UPTWANS_PushAttack::NotifyBegin(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation,
                                     float TotalDuration)
{
	if (APTWPlayerCharacter* Owner = Cast<APTWPlayerCharacter>(MeshComp->GetOwner()))
	{
		HitActors.Empty();
		Owner->GetPushCollision()->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
		
		TArray<AActor*> OverlappingActors;
		Owner->GetPushCollision()->GetOverlappingActors(OverlappingActors, APawn::StaticClass());
        
		for (AActor* Target : OverlappingActors)
		{
			ProcessHit(Owner, Target); 
		}
	}
}

void UPTWANS_PushAttack::NotifyTick(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation,
	float FrameDeltaTime)
{
	if (APTWPlayerCharacter* Owner = Cast<APTWPlayerCharacter>(MeshComp->GetOwner()))
	{
		TArray<AActor*> OverlappingActors;
		Owner->GetPushCollision()->GetOverlappingActors(OverlappingActors, APawn::StaticClass());
        
		for (AActor* Target : OverlappingActors)
		{
			if (!HitActors.Contains(Target))
			{
				ProcessHit(Owner, Target);
			}
		}
	}
}

void UPTWANS_PushAttack::NotifyEnd(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation)
{
	if (APTWPlayerCharacter* Owner = Cast<APTWPlayerCharacter>(MeshComp->GetOwner()))
	{
		Owner->GetPushCollision()->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	}
}

void UPTWANS_PushAttack::ProcessHit(AActor* Owner, AActor* Target)
{
	if (Target && Target != Owner)
	{
		HitActors.Add(Target);
        
		FGameplayEventData Payload;
		Payload.Target = Target;
		Payload.Instigator = Owner;
		UAbilitySystemBlueprintLibrary::SendGameplayEventToActor(Owner, GameplayTags::Event::Melee::Hit, Payload);
	}
}
