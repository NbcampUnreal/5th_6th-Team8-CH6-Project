// Fill out your copyright notice in the Description page of Project Settings.


#include "CoreFramework/Character/Component/PTWWeaponComponent.h"
#include "Net/UnrealNetwork.h"
#include "Inventory/PTWWeaponActor.h"
#include "Inventory/PTWWeaponData.h"
#include "GameFramework/Character.h"
#include "CoreFramework/PTWPlayerCharacter.h"
#include "Inventory/PTWInventoryComponent.h"

UPTWWeaponComponent::UPTWWeaponComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
	SetIsReplicatedByDefault(true);
}

void UPTWWeaponComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(UPTWWeaponComponent, CurrentWeaponTag);
	DOREPLIFETIME(UPTWWeaponComponent, CurrentWeapon);
}

void UPTWWeaponComponent::EquipWeaponByTag(FGameplayTag NewWeaponTag)
{
	if (GetOwnerRole() != ROLE_Authority) return;

	OnRep_CurrentWeaponTag(CurrentWeaponTag);

	if (CurrentWeaponTag == NewWeaponTag)
	{
		if (CurrentWeapon)
		{
			if (FWeaponPair* CurrentPair = SpawnedWeapons.Find(CurrentWeaponTag))
			{
				if (CurrentPair->Weapon1P) CurrentPair->Weapon1P->SetActorHiddenInGame(true);
				if (CurrentPair->Weapon3P) CurrentPair->Weapon3P->SetActorHiddenInGame(true);
			}
			CurrentWeapon = nullptr;
		}
		CurrentWeaponTag = FGameplayTag::EmptyTag;
		return;
	}

	// 기존 무기 숨기기
	if (CurrentWeaponTag.IsValid())
	{
		if (FWeaponPair* OldPair = SpawnedWeapons.Find(CurrentWeaponTag))
		{
			if (OldPair->Weapon1P) OldPair->Weapon1P->SetActorHiddenInGame(true);
			if (OldPair->Weapon3P) OldPair->Weapon3P->SetActorHiddenInGame(true);
		}
	}

	// 새 무기 보이기
	if (FWeaponPair* FoundPair = SpawnedWeapons.Find(NewWeaponTag))
	{
		if (FoundPair->Weapon1P && FoundPair->Weapon3P)
		{
			FoundPair->Weapon1P->SetActorHiddenInGame(false);
			FoundPair->Weapon3P->SetActorHiddenInGame(false);

			CurrentWeapon = FoundPair->Weapon1P;
			CurrentWeaponTag = NewWeaponTag;
		}
	}
}

void UPTWWeaponComponent::AttachWeaponToSocket(APTWWeaponActor* NewWeapon1P, APTWWeaponActor* NewWeapon3P, FGameplayTag WeaponTag)
{
	if (!NewWeapon1P || !NewWeapon3P) return;
	APTWPlayerCharacter* PlayerChar = Cast<APTWPlayerCharacter>(GetOwner());
	if (!PlayerChar) return;

	FWeaponPair Weaponpair;
	Weaponpair.Weapon1P = NewWeapon1P;
	Weaponpair.Weapon3P = NewWeapon3P;
	SpawnedWeapons.Add(WeaponTag, Weaponpair);

	// [이동] Character Mesh에 부착
	NewWeapon1P->AttachToComponent(PlayerChar->GetMesh1P(), FAttachmentTransformRules::SnapToTargetNotIncludingScale, TEXT("WeaponSocket"));
	NewWeapon3P->AttachToComponent(PlayerChar->GetMesh3P(), FAttachmentTransformRules::SnapToTargetNotIncludingScale, TEXT("WeaponSocket"));

	NewWeapon1P->ApplyVisualPerspective();
	NewWeapon3P->ApplyVisualPerspective();

	NewWeapon1P->SetActorHiddenInGame(true);
	NewWeapon3P->SetActorHiddenInGame(true);

	NewWeapon1P->SetActorEnableCollision(false);
	NewWeapon3P->SetActorEnableCollision(false);
}

void UPTWWeaponComponent::ApplyRecoil()
{
	if (!CurrentWeapon) return;
	const UPTWWeaponData* Data = CurrentWeapon->GetWeaponData();
	if (!Data) return;

	FGameplayTag FireTag = FGameplayTag::RequestGameplayTag(FName("Weapon.Anim.Fire"));
	if (Data->AnimMap.Contains(FireTag))
	{
		UAnimMontage* Montage = *Data->AnimMap.Find(FireTag);
		if (Montage)
		{
			PlayMontage1P(Montage);
		}
	}
}

float UPTWWeaponComponent::PlayMontage1P(UAnimMontage* MontageToPlay)
{
	APTWPlayerCharacter* PlayerChar = Cast<APTWPlayerCharacter>(GetOwner());
	if (!PlayerChar || !PlayerChar->IsLocallyControlled() || !PlayerChar->GetMesh1P() || !MontageToPlay)
	{
		return 0.0f;
	}

	UAnimInstance* AnimInstance = PlayerChar->GetMesh1P()->GetAnimInstance();
	if (AnimInstance)
	{
		return AnimInstance->Montage_Play(MontageToPlay, 1.0f);
	}
	return 0.0f;
}

void UPTWWeaponComponent::OnRep_CurrentWeaponTag(const FGameplayTag& OldTag)
{
	APTWPlayerCharacter* PlayerChar = Cast<APTWPlayerCharacter>(GetOwner());
	if (!PlayerChar) return;

	if (UPTWInventoryComponent* InvComp = PlayerChar->GetInventoryComponent())
	{
		if (OldTag != FGameplayTag::EmptyTag)
		{
			InvComp->WeaponVisibleSetting(OldTag, true);
		}
		InvComp->WeaponVisibleSetting(CurrentWeaponTag, false);
	}
}

void UPTWWeaponComponent::OnRep_CurrentWeapon(APTWWeaponActor* OldWeapon)
{

}

