// Fill out your copyright notice in the Description page of Project Settings.


#include "GC_WeaponFire.h"

#include "IDetailTreeNode.h"
#include "NiagaraFunctionLibrary.h"
#include "CoreFramework/PTWPlayerCharacter.h"
#include "Inventory/PTWItemInstance.h"
#include "Inventory/PTWWeaponActor.h"
#include "Kismet/GameplayStatics.h"


bool UGC_WeaponFire::OnExecute_Implementation(AActor* MyTarget, const FGameplayCueParameters& Parameters) const
{
	if (!MyTarget)
	{
		return false;
	}
	UObject* SourceObj = const_cast<UObject*>(Parameters.GetSourceObject());
	UPTWItemInstance* ItemInst = Cast<UPTWItemInstance>(SourceObj);
	
	APTWPlayerCharacter* PC = Cast<APTWPlayerCharacter>(MyTarget);
	
	if (!PC) return false;
	
	APTWWeaponActor* TargetWeapon = nullptr;
	
	// PC가 나라면 1인칭용 Muzzle
	// PC가 상대라면 3인칭용 Muzzle
	
	if (PC->IsLocallyControlled())
	{
		TargetWeapon = ItemInst->SpawnedWeapon1P;
	}
	else
	{
		TargetWeapon = ItemInst->SpawnedWeapon3P;
	}
	
	bool bIsHidden = TargetWeapon->IsHidden();
	bool bIsFirstPerson = TargetWeapon->bIsFirstPersonWeapon;
	
	UE_LOG(LogTemp, Log, TEXT("Hidden: %s"), bIsHidden ? TEXT("true") : TEXT("false"));
	UE_LOG(LogTemp, Log, TEXT("bIsFirstPerson: %s"), bIsFirstPerson ? TEXT("true") : TEXT("false"));
	
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
