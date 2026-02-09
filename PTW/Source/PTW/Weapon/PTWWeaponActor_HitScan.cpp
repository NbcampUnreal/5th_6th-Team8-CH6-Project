#include "PTWWeaponActor_HitScan.h"

#include "PTWWeaponCasting.h"


APTWWeaponActor_HitScan::APTWWeaponActor_HitScan()
{
	PrimaryActorTick.bCanEverTick = false;
	CastingSocket = CreateDefaultSubobject<UStaticMeshComponent>("CastingMeshComponent");
	CastingSocket->SetupAttachment(RootComponent);
}

void APTWWeaponActor_HitScan::SpawnCastingActor() const
{
	if (CastingClass)
	{
		GetWorld()->SpawnActor<APTWWeaponCasting>(
			CastingClass,
			CastingSocket->GetComponentLocation(),
			CastingSocket->GetComponentRotation()
			);
	}
}

void APTWWeaponActor_HitScan::BeginPlay()
{
	Super::BeginPlay();
	
}


