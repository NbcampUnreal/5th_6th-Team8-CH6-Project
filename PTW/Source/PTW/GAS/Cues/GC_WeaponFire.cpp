// Fill out your copyright notice in the Description page of Project Settings.


#include "GC_WeaponFire.h"

#include "NiagaraFunctionLibrary.h"
#include "CoreFramework/PTWPlayerCharacter.h"
#include "Inventory/PTWInventoryComponent.h"
#include "Inventory/PTWItemDefinition.h"
#include "Inventory/Instance/PTWItemInstance.h"
#include "Weapon/PTWWeaponActor.h"
#include "Inventory/Instance/PTWWeaponInstance.h"
#include "Kismet/GameplayStatics.h"
#include "Sound/SoundCue.h"
#include "Weapon/PTWWeaponActor_HitScan.h"
#include "Weapon/PTWWeaponSoundTable.h"


class UPTWWeaponInstance;

bool UGC_WeaponFire::OnExecute_Implementation(AActor* MyTarget, const FGameplayCueParameters& Parameters) const
{
	APTWPlayerCharacter* PC = Cast<APTWPlayerCharacter>(MyTarget);
	if (!PC) return false;
	
	const APTWWeaponActor* TargetWeapon = nullptr;
	
	if (PC->IsLocallyControlled())
	{
		TargetWeapon = Cast<APTWWeaponActor>(Parameters.SourceObject);
	}
	else
	{
		if (UPTWInventoryComponent* InvenComp = PC->GetInventoryComponent())
		{
			if (UPTWWeaponInstance* ItemInst = InvenComp->GetCurrentWeaponInst<UPTWWeaponInstance>())
			{
				TargetWeapon = ItemInst->SpawnedWeapon3P;
			}
		}
	}
	
	if (!TargetWeapon || !TargetWeapon->GetMuzzleComponent())
	{
		return false;
	}
	
	USceneComponent* MuzzleComp = TargetWeapon->GetMuzzleComponent();
	
	
	if (FireVFX)
	{
		UNiagaraFunctionLibrary::SpawnSystemAttached(
			FireVFX,
			MuzzleComp,
			NAME_None,
			FVector::ZeroVector,
			FRotator::ZeroRotator,
			EAttachLocation::SnapToTargetIncludingScale,
			true
		);
	}
	
	
	if (WeaponSoundTable)
	{
		FGameplayTagContainer WeaponTags = Parameters.AggregatedSourceTags;
		USoundCue* Cue = WeaponSoundTable->GetSoundForTag(TargetWeapon->GetWeaponItemInstance()->ItemDef->WeaponTag);
		
		if (Cue)
		{
			UGameplayStatics::PlaySoundAtLocation(
				GetWorld(), 
				Cue, 
				MuzzleComp->GetComponentLocation()
			);
		}
	}
	
	SpawnCasing(TargetWeapon);

	return true;
}

void UGC_WeaponFire::SpawnCasing(const APTWWeaponActor* TargetWeapon) const
{
	if (const APTWWeaponActor_HitScan* HitScanWeapon = Cast<APTWWeaponActor_HitScan>(TargetWeapon))
	{
		HitScanWeapon->SpawnCastingActor();
	}
}
