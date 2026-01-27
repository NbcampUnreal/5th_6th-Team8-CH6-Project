// Fill out your copyright notice in the Description page of Project Settings.


#include "PTWBaseCharacter.h"

#include "AbilitySystemBlueprintLibrary.h"
#include "Components/CapsuleComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "AbilitySystemComponent.h"
#include "GAS/PTWGameplayAbility.h"
#include "Inventory/PTWWeaponActor.h"
#include "Net/UnrealNetwork.h"
#include "AbilitySystemBlueprintLibrary.h"
#include "PTWPlayerController.h"

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
	
	DeadTag = FGameplayTag::RequestGameplayTag(FName("State.Status.Dead"));
}

void APTWBaseCharacter::BeginPlay()
{
	Super::BeginPlay();
	
}

UAbilitySystemComponent* APTWBaseCharacter::GetAbilitySystemComponent() const
{
	return AbilitySystemComponent;
}

bool APTWBaseCharacter::IsDead() const
{
	return AbilitySystemComponent->HasMatchingGameplayTag(DeadTag);
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

void APTWBaseCharacter::HandleDeath(AActor* Attacker)
{
	if (!HasAuthority() || !AbilitySystemComponent)
	{
		return;
	}
	FGameplayEventData Payload;
	// Payload.EventTag = FGameplayTag::RequestGameplayTag(FName("죽음 이벤트 연결"));
	Payload.Instigator = Attacker;
	Payload.Target = this;
	// UAbilitySystemBlueprintLibrary::SendGameplayEventToActor(this, Payload.EventTag, Payload);

	GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, 
		FString::Printf(TEXT("%s가 %s를 죽임"), *Attacker->GetName(), *GetName())
		);
}



