#include "PTWWeaponInstance.h"

#include "AbilitySystemBlueprintLibrary.h"
#include "AbilitySystemComponent.h"
#include "CoreFramework/PTWPlayerCharacter.h"
#include "GAS/PTWWeaponAttributeSet.h"
#include "Inventory/PTWInventoryComponent.h"
#include "Net/UnrealNetwork.h"


void UPTWWeaponInstance::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(UPTWWeaponInstance, CurrentAmmo);
	DOREPLIFETIME(UPTWWeaponInstance, SpawnedWeapon1P);
	DOREPLIFETIME(UPTWWeaponInstance, SpawnedWeapon3P);
	DOREPLIFETIME(UPTWWeaponInstance, bAlreadyUsing);
}

void UPTWWeaponInstance::OnRep_CurrentAmmo()
{
	
}

void UPTWWeaponInstance::OnRep_SpawnedWeapon()
{
	if (SpawnedWeapon1P && SpawnedWeapon1P->GetWeaponData())
	{
		CurrentAmmo = SpawnedWeapon1P->GetWeaponData()->MaxAmmo;
	}
}

void UPTWWeaponInstance::OnRep_SpawnedWeapon3P()
{
	
}


void UPTWWeaponInstance::DestroySpawnedActors()
{
	if (SpawnedWeapon1P && SpawnedWeapon3P)
	{
		SpawnedWeapon1P->Destroy();
		SpawnedWeapon3P->Destroy();
	
		SpawnedWeapon1P = nullptr;
		SpawnedWeapon3P = nullptr;
	}
}

void UPTWWeaponInstance::SetCurrentAmmo(int32 NewAmmo)
{
	int32 MaxAmmo = GetMaxAmmo();
	// 값의 범위를 제한하고 변경된 경우에만 방송
	int32 ClampedAmmo = FMath::Clamp(NewAmmo, 0, MaxAmmo);

	if (CurrentAmmo != ClampedAmmo)
	{
		CurrentAmmo = ClampedAmmo;
		OnAmmoChanged.Broadcast(CurrentAmmo, MaxAmmo);
	}
}

int32 UPTWWeaponInstance::GetMaxAmmo()
{
	if (APTWPlayerCharacter* PlayerCharacter = GetItemInstanceOwner())
	{
		if (UAbilitySystemComponent* ASC = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(PlayerCharacter))
		{
			return ASC->GetNumericAttribute(UPTWWeaponAttributeSet::GetMaxAmmoAttribute());
		}
	}
	return 0;
}

int32 UPTWWeaponInstance::GetCurrentAmmo()
{
	if (APTWPlayerCharacter* PlayerCharacter = GetItemInstanceOwner())
	{
		if (UAbilitySystemComponent* ASC = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(PlayerCharacter))
		{
			return ASC->GetNumericAttribute(UPTWWeaponAttributeSet::GetCurrentAmmoAttribute());
		}
	}
	return 0;
}

APTWPlayerCharacter* UPTWWeaponInstance::GetItemInstanceOwner()
{
	UPTWInventoryComponent* OwnerComp = Cast<UPTWInventoryComponent>(GetOuter());
	
	if (OwnerComp)
	{
		APTWPlayerState* PS = Cast<APTWPlayerState>(OwnerComp->GetOwner());
		if (!PS) return nullptr;
		
		return Cast<APTWPlayerCharacter>(PS->GetPawn());
	}
	
	return nullptr;
}

void UPTWWeaponInstance::CopyProperties(UPTWWeaponInstance& CopyInst)
{
	CurrentAmmo = CopyInst.GetWeaponData()->MaxAmmo;
	bAlreadyUsing = CopyInst.bAlreadyUsing;
}
