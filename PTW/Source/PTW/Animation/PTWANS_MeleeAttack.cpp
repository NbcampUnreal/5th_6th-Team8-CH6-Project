// Fill out your copyright notice in the Description page of Project Settings.


#include "PTWANS_MeleeAttack.h"

#include "AbilitySystemBlueprintLibrary.h"
#include "AbilitySystemComponent.h"
#include "PTW.h"
#include "CoreFramework/PTWPlayerCharacter.h"
#include "CoreFramework/Character/Component/PTWWeaponComponent.h"
#include "Inventory/PTWInventoryComponent.h"
#include "Inventory/Instance/PTWWeaponInstance.h"
#include "PTWGameplayTag/GameplayTags.h"
#include "Weapon/PTWWeaponActor.h"

void UPTWANS_MeleeAttack::NotifyBegin(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation,
                                      float TotalDuration)
{
	HitActors.Empty();
	
	Owner = Cast<APTWPlayerCharacter>(MeshComp->GetOwner());
	if (!Owner) return;
	
	UPTWInventoryComponent* InvenComp = Owner->GetInventoryComponent();
	if (!InvenComp) return;
	
	UPTWWeaponInstance* WeaponInst =  InvenComp->GetCurrentWeaponInst<UPTWWeaponInstance>();
	if (!WeaponInst) return;
	
	MeleeWeapon = WeaponInst->SpawnedWeapon3P;
}

void UPTWANS_MeleeAttack::NotifyTick(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation,
	float FrameDeltaTime)
{
	FVector StartPos;
	FVector EndPos;
	
	if (MeleeWeapon)
	{
		StartPos = MeleeWeapon->GetWeaponMesh()->GetSocketLocation(TEXT("StartPos"));
		EndPos = MeleeWeapon->GetWeaponMesh()->GetSocketLocation(TEXT("EndPos"));
	}
	
	float SphereRad = 20.0f;
	TArray<FHitResult> OutHits;
	
	FCollisionQueryParams CollisionParams;
	CollisionParams.AddIgnoredActor(Owner);
	CollisionParams.AddIgnoredActor(MeleeWeapon);
	
	bool bHit = Owner->GetWorld()->SweepMultiByChannel(
		OutHits,
		StartPos,
		EndPos,
		FQuat::Identity,
		ECC_WeaponAttack, 
		FCollisionShape::MakeSphere(SphereRad),
		CollisionParams
	);
	
	DrawDebugCapsule(Owner->GetWorld(), (StartPos + EndPos) * 0.5f, 
	    FVector::Dist(StartPos, EndPos) * 0.5f + SphereRad, SphereRad, 
	    FRotationMatrix::MakeFromZ(EndPos - StartPos).ToQuat(), FColor::Red, false, 1.0f);
	
	if (bHit)
	{
		for (const FHitResult& Hit : OutHits)
		{
			AActor* HitActor = Hit.GetActor();
			if (HitActor && !HitActors.Contains(HitActor))
			{
				HitActors.Add(HitActor);
				
				FGameplayEventData Payload;
				Payload.Target = HitActor;
				Payload.Instigator = Owner;
				Payload.TargetData = UAbilitySystemBlueprintLibrary::AbilityTargetDataFromHitResult(Hit);

				UAbilitySystemBlueprintLibrary::SendGameplayEventToActor(Owner, GameplayTags::Event::Melee::Hit, Payload);
			}
		}
	}
	
	
	
}

void UPTWANS_MeleeAttack::NotifyEnd(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation)
{
	
}
