// Fill out your copyright notice in the Description page of Project Settings.


#include "GC_WeaponFire.h"

#include "IDetailTreeNode.h"
#include "NiagaraFunctionLibrary.h"
#include "CoreFramework/PTWPlayerCharacter.h"
#include "Inventory/PTWInventoryComponent.h"
#include "Inventory/PTWItemInstance.h"
#include "Inventory/PTWWeaponActor.h"
#include "Kismet/GameplayStatics.h"


bool UGC_WeaponFire::OnExecute_Implementation(AActor* MyTarget, const FGameplayCueParameters& Parameters) const
{
	if (!MyTarget)
	{
		return false;
	}
	
	const APTWWeaponActor* SourceWeapon = Cast<APTWWeaponActor>(Parameters.SourceObject);
	const APTWWeaponActor* TargetWeapon = nullptr;
	
	APTWPlayerCharacter* PC = Cast<APTWPlayerCharacter>(MyTarget);
	
	if (!PC) return false;
	
	
	if (PC->IsLocallyControlled())
	{
		TargetWeapon = SourceWeapon;
	}
	else
	{
		// 서버기준으로 서버에 해당하는 PC의 인벤토리 참조해서 가져오기
		if (UPTWInventoryComponent* InvenComp = PC->GetInventoryComponent())
		{
			if (UPTWItemInstance* ItemInstServer = InvenComp->GetCurrentWeaponInst())
			{
				TargetWeapon = ItemInstServer->SpawnedWeapon3P;
			}
		}
	}
	
	UNiagaraFunctionLibrary::SpawnSystemAttached(
		FireVFX,
		TargetWeapon->GetMuzzleComponent(),
		NAME_None,
		FVector::ZeroVector,
		FRotator::ZeroRotator, 
		EAttachLocation::SnapToTargetIncludingScale,
		true
		);
	
	if (FireSFX)
	{
		UGameplayStatics::PlaySoundAtLocation(GetWorld(), FireSFX, MyTarget->GetActorLocation());
	}
	
	return true;
	
}
