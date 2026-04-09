// Fill out your copyright notice in the Description page of Project Settings.


#include "CoreFramework/Character/Component/PTWWeaponComponent.h"
#include "Net/UnrealNetwork.h"
#include "Weapon/PTWWeaponActor.h"
#include "Weapon/PTWWeaponData.h"
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
	if (CurrentWeaponTag.IsValid())
	{
		if (FWeaponPair* OldPair = SpawnedWeapons.Find(CurrentWeaponTag))
		{
			if (OldPair->Weapon1P) OldPair->Weapon1P->SetActorHiddenInGame(true);
			if (OldPair->Weapon3P) OldPair->Weapon3P->SetActorHiddenInGame(true);
		}
	}
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

UAnimMontage* UPTWWeaponComponent::PlayWeaponMontages(FGameplayTag AnimTag, bool bAutoPlayCharacterMontage)
{
	if (!CurrentWeapon) return nullptr;

	const UPTWWeaponData* Data = CurrentWeapon->GetWeaponData();
	if (!Data) return nullptr;

	if (Data->WeaponAnimMap.Contains(AnimTag))
	{
		UAnimMontage* WeaponMontage = *Data->WeaponAnimMap.Find(AnimTag);
		if (WeaponMontage)
		{
			CurrentWeapon->PlayWeaponMontage(WeaponMontage);
		}
	}

	if (Data->AnimMap.Contains(AnimTag))
	{
		UAnimMontage* CharacterMontage = *Data->AnimMap.Find(AnimTag);
		if (CharacterMontage)
		{
			if (bAutoPlayCharacterMontage)
			{
				PlayCharacterMontage1P(CharacterMontage);
				return nullptr;
			}
			else
			{
				return CharacterMontage;
			}
		}
	}

	return nullptr;
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

	FName TargetSocketName = TEXT("WeaponSocket");

	if (const UPTWWeaponData* WeaponData = NewWeapon1P->GetWeaponData())
	{
		TargetSocketName = WeaponData->AttachSocketName; 
	}

	NewWeapon1P->AttachToComponent(PlayerChar->GetMesh1P(), FAttachmentTransformRules::SnapToTargetNotIncludingScale, TargetSocketName);
	NewWeapon3P->AttachToComponent(PlayerChar->GetMesh3P(), FAttachmentTransformRules::SnapToTargetNotIncludingScale, TargetSocketName);

	NewWeapon1P->ApplyVisualPerspective();
	NewWeapon3P->ApplyVisualPerspective();

	NewWeapon1P->SetActorHiddenInGame(true);
	NewWeapon3P->SetActorHiddenInGame(true);

	NewWeapon1P->SetActorEnableCollision(false);
	NewWeapon3P->SetActorEnableCollision(false);
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

float UPTWWeaponComponent::PlayCharacterMontage1P(UAnimMontage* MontageToPlay)
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
