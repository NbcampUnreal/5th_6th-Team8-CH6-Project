// Fill out your copyright notice in the Description page of Project Settings.


#include "PTWBaseCharacter.h"
#include "Components/CapsuleComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "AbilitySystemComponent.h"
#include "GAS/PTWGameplayAbility.h"
#include "Inventory/PTWWeaponActor.h"

APTWBaseCharacter::APTWBaseCharacter()
{
	PrimaryActorTick.bCanEverTick = false;

	bUseControllerRotationPitch = false;
	bUseControllerRotationYaw = false;
	bUseControllerRotationRoll = false;

	GetCharacterMovement()->bOrientRotationToMovement = true;
	GetCharacterMovement()->RotationRate = FRotator(0.0f, 500.0f, 0.0f);
	GetCharacterMovement()->JumpZVelocity = 420.f;
	GetCharacterMovement()->AirControl = 0.35f;
	GetCharacterMovement()->MaxWalkSpeed = 500.f;

	AbilitySystemComponent = CreateDefaultSubobject<UAbilitySystemComponent>(TEXT("AbilitySystemComponent"));
	AbilitySystemComponent->SetIsReplicated(true);
	AbilitySystemComponent->SetReplicationMode(EGameplayEffectReplicationMode::Mixed);
}

void APTWBaseCharacter::BeginPlay()
{
	Super::BeginPlay();

	if (GetWorld())
	{
		for (const auto& Pair : WeaponClasses)
		{
			FGameplayTag Tag = Pair.Key;
			TSubclassOf<APTWWeaponActor> ClassToSpawn = Pair.Value;

			if (ClassToSpawn)
			{
				FActorSpawnParameters Params;
				Params.Owner = this;

				APTWWeaponActor* NewWeapon = GetWorld()->SpawnActor<APTWWeaponActor>(ClassToSpawn, Params);

				if (NewWeapon)
				{
					NewWeapon->AttachToComponent(GetMesh(), FAttachmentTransformRules::SnapToTargetNotIncludingScale, FName("WeaponSocket"));

					NewWeapon->SetActorHiddenInGame(true);
					NewWeapon->SetActorEnableCollision(false);

					SpawnedWeapons.Add(Tag, NewWeapon);
				}
			}
		}
	}
}

UAbilitySystemComponent* APTWBaseCharacter::GetAbilitySystemComponent() const
{
	return AbilitySystemComponent;
}

void APTWBaseCharacter::InitAbilityActorInfo()
{

}

void APTWBaseCharacter::GiveDefaultAbilities()
{
	if (!HasAuthority() || !AbilitySystemComponent) return;

	for (TSubclassOf<UGameplayAbility> AbilityClass : DefaultAbilities)
	{
		if (AbilityClass)
		{
			FGameplayAbilitySpec Spec(AbilityClass, 1, INDEX_NONE, this);

			if (const UPTWGameplayAbility* PTWAbility = Cast<UPTWGameplayAbility>(AbilityClass->GetDefaultObject()))
			{
				if (PTWAbility->StartupInputTag.IsValid())
				{
					Spec.DynamicAbilityTags.AddTag(PTWAbility->StartupInputTag);
				}
			}

			AbilitySystemComponent->GiveAbility(Spec);
		}
	}
}

void APTWBaseCharacter::ApplyDefaultEffects()
{
	if (GetLocalRole() != ROLE_Authority || !AbilitySystemComponent) return;

	FGameplayEffectContextHandle ContextHandle = AbilitySystemComponent->MakeEffectContext();
	ContextHandle.AddSourceObject(this);

	for (TSubclassOf<UGameplayEffect> EffectClass : DefaultEffects)
	{
		if (!EffectClass) continue;

		FGameplayEffectSpecHandle SpecHandle = AbilitySystemComponent->MakeOutgoingSpec(EffectClass, 1.0f, ContextHandle);

		if (SpecHandle.IsValid())
		{
			AbilitySystemComponent->ApplyGameplayEffectSpecToSelf(*SpecHandle.Data.Get());
		}
	}
}

void APTWBaseCharacter::EquipWeaponByTag(FGameplayTag NewWeaponTag)
{
	if (CurrentWeaponTag == NewWeaponTag)
	{
		if (CurrentWeapon)
		{
			CurrentWeapon->SetActorHiddenInGame(true);
			CurrentWeapon->SetActorEnableCollision(false);
		}

		CurrentWeapon = nullptr;
		CurrentWeaponTag = FGameplayTag::EmptyTag;

		UE_LOG(LogTemp, Log, TEXT("Weapon Unequipped (Toggle Off)"));
		return;
	}

	if (CurrentWeapon)
	{
		CurrentWeapon->SetActorHiddenInGame(true);
		CurrentWeapon->SetActorEnableCollision(false);
	}

	if (APTWWeaponActor** FoundWeaponPtr = SpawnedWeapons.Find(NewWeaponTag))
	{
		APTWWeaponActor* NewWeaponActor = *FoundWeaponPtr;

		if (NewWeaponActor)
		{
			NewWeaponActor->SetActorHiddenInGame(false);

			CurrentWeapon = NewWeaponActor;
			CurrentWeaponTag = NewWeaponTag;


			UE_LOG(LogTemp, Log, TEXT("Weapon Equipped: %s"), *NewWeaponTag.ToString());
		}
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("Cannot find weapon with tag: %s"), *NewWeaponTag.ToString());
	}
}
