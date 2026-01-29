
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
		// 1인칭 무기 세팅
		WeaponMesh->SetOnlyOwnerSee(true);
		WeaponMesh->SetOwnerNoSee(false);
		if (!bIsLocal)
		{
			WeaponMesh->SetCastShadow(false);
		}
	}
	else
	{
		// 3인칭 무기 세팅
		if (bIsLocal)
		{
			WeaponMesh->SetOwnerNoSee(true);
			WeaponMesh->SetCastShadow(false);
		} 
		else
		{
			WeaponMesh->SetOnlyOwnerSee(false);
			WeaponMesh->SetOwnerNoSee(false);
			WeaponMesh->SetVisibility(true);
		}
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

