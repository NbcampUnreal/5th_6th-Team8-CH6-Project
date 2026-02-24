#include "PTWPickupWeapon.h"

#include "AbilitySystemBlueprintLibrary.h"
#include "CoreFramework/PTWPlayerCharacter.h"
#include "Engine/ActorChannel.h"
#include "Inventory/PTWInventoryComponent.h"
#include "Inventory/PTWItemDefinition.h"
#include "Inventory/Instance/PTWItemInstance.h"
#include "Inventory/Instance/PTWWeaponInstance.h"
#include "Net/UnrealNetwork.h"
#include "PTWGameplayTag/GameplayTags.h"
#include "System/PTWItemSpawnManager.h"

APTWPickupWeapon::APTWPickupWeapon()
{
	
}

void APTWPickupWeapon::SetWeaponInstance(UPTWWeaponInstance* Inst)
{
	WeaponInstance = Inst;
	UpdatingWeaponMesh();
}

bool APTWPickupWeapon::ReplicateSubobjects(class UActorChannel* Channel, class FOutBunch* Bunch,
                                           FReplicationFlags* RepFlags)
{
	bool WroteSomething = Super::ReplicateSubobjects(Channel, Bunch, RepFlags);
	if (WeaponInstance)
	{
		WroteSomething |= Channel->ReplicateSubobject(WeaponInstance, *Bunch, *RepFlags);
	}
	return WroteSomething;
}

void APTWPickupWeapon::GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(ThisClass, WeaponInstance);
}

void APTWPickupWeapon::BeginPlay()
{
	Super::BeginPlay();
}

void APTWPickupWeapon::OnPickedUp(class APTWPlayerCharacter* Player)
{
	if (!HasAuthority() || !Player || !WeaponInstance) return;
	
	if (UPTWInventoryComponent* InvenComp = Player->GetInventoryComponent())
	{
		WeaponInstance->Rename(nullptr, InvenComp);
		
		if (UPTWItemSpawnManager* SpawnManager = GetWorld()->GetSubsystem<UPTWItemSpawnManager>())
		{
			SpawnManager->AddPickupWeapon(WeaponInstance, Player);
			WeaponInstance = nullptr;
			Destroy(); 
		}
	}
}

void APTWPickupWeapon::UpdatingWeaponMesh()
{
	if (WeaponInstance)
	{
		FGameplayTag RifleTag = GameplayTags::Weapon::Gun::Rifle::Rifle;
		FGameplayTag PistolTag = GameplayTags::Weapon::Gun::Pistol::Pistol;
		if (WeaponInstance->ItemDef->WeaponTag.MatchesTag(RifleTag))
		{
			MeshComp->SetStaticMesh(WeaponMesh[0]);
		}
		else if (WeaponInstance->ItemDef->WeaponTag.MatchesTag(PistolTag))
		{
			MeshComp->SetStaticMesh(WeaponMesh[1]);
		}
	}
}


