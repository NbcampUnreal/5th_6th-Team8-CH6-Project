#include "PTWPickupWeapon.h"

#include "CoreFramework/PTWPlayerCharacter.h"
#include "Engine/ActorChannel.h"
#include "Inventory/PTWInventoryComponent.h"
#include "Inventory/Instance/PTWItemInstance.h"
#include "Inventory/Instance/PTWWeaponInstance.h"
#include "Net/UnrealNetwork.h"
#include "System/PTWItemSpawnManager.h"

APTWPickupWeapon::APTWPickupWeapon()
{
	
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


