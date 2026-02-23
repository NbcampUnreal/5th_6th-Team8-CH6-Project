
#include "PTWWeaponActor.h"

#include "Components/SphereComponent.h"
#include "CoreFramework/PTWPlayerCharacter.h"
#include "Engine/ActorChannel.h"
#include "Inventory/Instance/PTWWeaponInstance.h"
#include "Net/UnrealNetwork.h"



APTWWeaponActor::APTWWeaponActor()
{
	PrimaryActorTick.bCanEverTick = false;
	bReplicates = true;
	
	RootScene = CreateDefaultSubobject<USceneComponent>(TEXT("RootSceneComponent"));
	SetRootComponent(RootScene);
	
	MuzzleSocket = CreateDefaultSubobject<USceneComponent>(TEXT("MuzzleSocket"));
	MuzzleSocket->SetupAttachment(RootComponent);
	
	WeaponMesh = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("WeaponMesh"));
	WeaponMesh->SetupAttachment(RootComponent);
	
	SphereComponent = CreateDefaultSubobject<USphereComponent>(TEXT("SphereComponent"));
	SphereComponent->SetupAttachment(RootComponent);
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
			WeaponMesh->SetOnlyOwnerSee(false);
			WeaponMesh->SetCastShadow(false);
		} 
		else
		{
			WeaponMesh->SetOnlyOwnerSee(false);
			WeaponMesh->SetOwnerNoSee(true);
		}
	}
}

void APTWWeaponActor::GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(APTWWeaponActor, bIsFirstPersonWeapon);
	DOREPLIFETIME(APTWWeaponActor, WeaponItemInstance);
	DOREPLIFETIME(APTWWeaponActor, bIsDrop);
}

void APTWWeaponActor::SetFirstPersonMode(bool bIsFirstPerson)
{
	if (GetLocalRole() == ROLE_Authority)
	{
		bIsFirstPersonWeapon = bIsFirstPerson;
		OnRep_IsFirstPersonWeapon();
	}
}

void APTWWeaponActor::BeginPlay()
{
	Super::BeginPlay();
	SetDroppingMode();
	ApplyVisualPerspective();
}

void APTWWeaponActor::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	if (WeaponItemInstance)
	{
		WeaponItemInstance->SpawnedWeapon1P = nullptr;
		WeaponItemInstance->SpawnedWeapon3P = nullptr;
	}
	
	Super::EndPlay(EndPlayReason);
}

bool APTWWeaponActor::ReplicateSubobjects(class UActorChannel* Channel, class FOutBunch* Bunch,
	FReplicationFlags* RepFlags)
{
	bool WroteSomething = Super::ReplicateSubobjects(Channel, Bunch, RepFlags);
	
	if (WeaponItemInstance)
	{
		WroteSomething |= Channel->ReplicateSubobject(WeaponItemInstance, *Bunch, *RepFlags);
	}
	
	return WroteSomething;
}


void APTWWeaponActor::OnRep_IsFirstPersonWeapon()
{
	ApplyVisualPerspective();
}

void APTWWeaponActor::OnRep_IsDrop()
{
	SetDroppingMode();
}

float APTWWeaponActor::PlayWeaponMontage(UAnimMontage* MontageToPlay)
{
	if (WeaponMesh && MontageToPlay)
	{
		UAnimInstance* AnimInstance = WeaponMesh->GetAnimInstance();
		if (AnimInstance)
		{
			return AnimInstance->Montage_Play(MontageToPlay);
		}
	}
	return 0.0f;
}

void APTWWeaponActor::SetWeaponItemInstance(UPTWWeaponInstance* ItemInstance)
{
	this->WeaponItemInstance = ItemInstance;
}

void APTWWeaponActor::HandleReloadEvent(EReloadEventAction ActionType)
{
	switch (ActionType)
	{
	case EReloadEventAction::DropMag:
		DropMag();
		break;
	case EReloadEventAction::GrabMag:
		GrabMag();
		break;
	case EReloadEventAction::InsertMag:
		InsertMag();
		break;
	}
}

void APTWWeaponActor::DropMag()
{
	if (EmptyMagazineClass)
	{
		FTransform MagTransform = WeaponMesh ? WeaponMesh->GetSocketTransform(MagBoneName) : GetActorTransform();

		FActorSpawnParameters SpawnParams;
		SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

		GetWorld()->SpawnActor<AActor>(EmptyMagazineClass, MagTransform, SpawnParams);
	}
}

void APTWWeaponActor::GrabMag()
{
	if (CurrentFakeMag)
	{
		CurrentFakeMag->Destroy();
		CurrentFakeMag = nullptr;
	}

	if (!MagazineClass) return;

	ACharacter* OwnerCharacter = Cast<ACharacter>(GetOwner());
	if (!OwnerCharacter) return;

	FActorSpawnParameters SpawnParams;
	SpawnParams.Owner = this;
	SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

	CurrentFakeMag = GetWorld()->SpawnActor<AActor>(MagazineClass, FVector::ZeroVector, FRotator::ZeroRotator, SpawnParams);

	if (CurrentFakeMag)
	{
		USkeletalMeshComponent* CharacterMesh = nullptr;

		if (APTWPlayerCharacter* PTWChar = Cast<APTWPlayerCharacter>(OwnerCharacter))
		{
			CharacterMesh = bIsFirstPersonWeapon ? PTWChar->GetMesh1P() : PTWChar->GetMesh3P();
		}
		else
		{
			CharacterMesh = OwnerCharacter->GetMesh();
		}

		if (CharacterMesh)
		{
			CurrentFakeMag->AttachToComponent(CharacterMesh, FAttachmentTransformRules::SnapToTargetNotIncludingScale, HandSocketName);
		}
	}
}

void APTWWeaponActor::InsertMag()
{
	if (CurrentFakeMag)
	{
		CurrentFakeMag->Destroy();
		CurrentFakeMag = nullptr;
	}
}

void APTWWeaponActor::SetDroppingMode()
{
	if (!bIsDrop)
	{
		SphereComponent->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	}
	else
	{
		
	}
}
