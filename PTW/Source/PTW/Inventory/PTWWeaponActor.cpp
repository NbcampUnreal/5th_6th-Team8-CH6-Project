
#include "PTWWeaponActor.h"

#include "AbilitySystemComponent.h"
#include "PTWWeaponData.h"
#include "CoreFramework/PTWBaseCharacter.h"
#include "Net/UnrealNetwork.h"


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

/*
 * 무기 메쉬 설정
 */
void APTWWeaponActor::ApplyVisualPerspective()
{
	APawn* OwningPawn = Cast<APawn>(GetOwner());
	if (!OwningPawn || !WeaponMesh) return;

	bool bIsLocal = OwningPawn->IsLocallyControlled();

	if (bIsFirstPersonWeapon)
	{
		// 1P 무기: 로컬 플레이어에게만 보임
		WeaponMesh->SetOnlyOwnerSee(true);
		WeaponMesh->SetOwnerNoSee(false);
	}
	else
	{
		// 3P 무기: 로컬 플레이어에게는 숨기고 타인에게만 보임
		WeaponMesh->SetOnlyOwnerSee(false);
		WeaponMesh->SetOwnerNoSee(true);
		WeaponMesh->SetCastHiddenShadow(true);
	}
}

void APTWWeaponActor::GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(APTWWeaponActor, bIsFirstPersonWeapon);
}

void APTWWeaponActor::BeginPlay()
{
	Super::BeginPlay();
	ApplyVisualPerspective();
}


void APTWWeaponActor::OnRep_IsFirstPersonWeapon()
{
	ApplyVisualPerspective();
}

