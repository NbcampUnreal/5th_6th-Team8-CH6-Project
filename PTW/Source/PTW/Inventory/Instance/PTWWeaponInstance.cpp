#include "PTWWeaponInstance.h"

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
	CurrentAmmo = SpawnedWeapon1P->GetWeaponData()->MaxAmmo;
}

void UPTWWeaponInstance::OnRep_SpawnedWeapon3P()
{
	
}

void UPTWWeaponInstance::DestroySpawnedActors()
{
	SpawnedWeapon1P->Destroy();
	SpawnedWeapon3P->Destroy();
	
	SpawnedWeapon1P = nullptr;
	SpawnedWeapon3P = nullptr;
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
	if (SpawnedWeapon1P && SpawnedWeapon1P->GetWeaponData())
	{
		return SpawnedWeapon1P->GetWeaponData()->MaxAmmo;
	}
	return 0;
}
