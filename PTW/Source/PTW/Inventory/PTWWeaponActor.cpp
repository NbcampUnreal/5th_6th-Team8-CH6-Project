
#include "PTWWeaponActor.h"

#include "AbilitySystemComponent.h"
#include "PTWWeaponData.h"
#include "CoreFramework/PTWBaseCharacter.h"


APTWWeaponActor::APTWWeaponActor()
{
	PrimaryActorTick.bCanEverTick = false;
	bReplicates = true;
	
	RootScene = CreateDefaultSubobject<USceneComponent>(TEXT("RootSceneComponent"));
	SetRootComponent(RootScene);
	
	MuzzleSocket = CreateDefaultSubobject<USceneComponent>(TEXT("MuzzleSocket"));
	MuzzleSocket->SetupAttachment(RootComponent);
	
	WeaponMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("WeaponMesh"));
	WeaponMesh->SetupAttachment(RootComponent);
}

void APTWWeaponActor::BeginPlay()
{
	Super::BeginPlay();
}

