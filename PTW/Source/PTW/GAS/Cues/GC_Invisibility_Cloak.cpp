#include "GC_Invisibility_Cloak.h"

#include "CoreFramework/PTWPlayerCharacter.h"
#include "Inventory/PTWInventoryComponent.h"
#include "Inventory/Instance/PTWWeaponInstance.h"


AGC_Invisibility_Cloak::AGC_Invisibility_Cloak()
{
	PrimaryActorTick.bCanEverTick = false;
}

bool AGC_Invisibility_Cloak::WhileActive_Implementation(AActor* MyTarget, const FGameplayCueParameters& Parameters)
{
	OwnerCharacter = Cast<APTWPlayerCharacter>(MyTarget);
	
	if (!OwnerCharacter) return false;
	
	UPTWInventoryComponent* TargetInventoryComp = OwnerCharacter->GetInventoryComponent();
	if (!TargetInventoryComp) return false;
	
	if (!SetPlayer3PMesh(true)) return false;
	SetPlayer3PWeaponMesh(true);
	
	return Super::WhileActive_Implementation(MyTarget, Parameters);
}

bool AGC_Invisibility_Cloak::OnRemove_Implementation(AActor* MyTarget, const FGameplayCueParameters& Parameters)
{
	if (!SetPlayer3PMesh(false)) return false;
	SetPlayer3PWeaponMesh(false);
	return Super::OnRemove_Implementation(MyTarget, Parameters);
}

bool AGC_Invisibility_Cloak::SetPlayer3PMesh(bool bIsHidden)
{
	if (!OwnerCharacter) return false;
	
	if (USkeletalMeshComponent* Mesh3P = OwnerCharacter->GetMesh3P())
	{
		Mesh3P->SetHiddenInGame(bIsHidden, true);
	}
	
	return true;
}

void AGC_Invisibility_Cloak::SetPlayer3PWeaponMesh(bool bIsHidden)
{
	if (ItemInstnace)
	{
		if (UPTWWeaponInstance* WeaponInstance = Cast<UPTWWeaponInstance>(ItemInstnace))
		{
			WeaponInstance->SpawnedWeapon3P->SetActorHiddenInGame(bIsHidden);
		}	
	}
}


