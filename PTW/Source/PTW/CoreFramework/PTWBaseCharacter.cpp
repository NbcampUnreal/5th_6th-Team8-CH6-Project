// Fill out your copyright notice in the Description page of Project Settings.


#include "PTWBaseCharacter.h"
#include "Components/CapsuleComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "AbilitySystemComponent.h"
#include "GAS/PTWGameplayAbility.h"

APTWBaseCharacter::APTWBaseCharacter()
{
	PrimaryActorTick.bCanEverTick = false;

	bUseControllerRotationPitch = false;
	bUseControllerRotationYaw = false;
	bUseControllerRotationRoll = false;

	GetCharacterMovement()->bOrientRotationToMovement = true;
	GetCharacterMovement()->RotationRate = FRotator(0.0f, 500.0f, 0.0f);
	GetCharacterMovement()->JumpZVelocity = 300.f;
	GetCharacterMovement()->AirControl = 0.35f;
	GetCharacterMovement()->MaxWalkSpeed = 420.f;

	AbilitySystemComponent = CreateDefaultSubobject<UAbilitySystemComponent>(TEXT("AbilitySystemComponent"));
	AbilitySystemComponent->SetIsReplicated(true);
	AbilitySystemComponent->SetReplicationMode(EGameplayEffectReplicationMode::Mixed);
}

void APTWBaseCharacter::BeginPlay()
{
	Super::BeginPlay();

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

