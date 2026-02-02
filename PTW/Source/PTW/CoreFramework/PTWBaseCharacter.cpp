// Fill out your copyright notice in the Description page of Project Settings.


#include "PTWBaseCharacter.h"
#include "Net/UnrealNetwork.h"
#include "AbilitySystemComponent.h"
#include "Kismet/KismetMathLibrary.h"
#include "GameFramework/PlayerState.h" 
#include "Components/CapsuleComponent.h"
#include "AbilitySystemBlueprintLibrary.h"
#include "GameFramework/CharacterMovementComponent.h"

#include "PTWPlayerState.h"
#include "PTWPlayerController.h"
#include "GAS/PTWGameplayAbility.h"
#include "Game/GameState/PTWGameState.h"
#include "CoreFramework/Character/Component/PTWReactorComponent.h"


APTWBaseCharacter::APTWBaseCharacter()
{
	PrimaryActorTick.bCanEverTick = false;

	AbilitySystemComponent = CreateDefaultSubobject<UAbilitySystemComponent>(TEXT("AbilitySystemComponent"));
	AbilitySystemComponent->SetIsReplicated(true);
	AbilitySystemComponent->SetReplicationMode(EGameplayEffectReplicationMode::Mixed);

	ReactorComponent = CreateDefaultSubobject<UPTWReactorComponent>(TEXT("ReactorComponent"));
	
	DeadTag = FGameplayTag::RequestGameplayTag(FName("State.Status.Dead"));
}

bool APTWBaseCharacter::IsDead() const
{
	return AbilitySystemComponent->HasMatchingGameplayTag(DeadTag);
}

UAbilitySystemComponent* APTWBaseCharacter::GetAbilitySystemComponent() const
{
	return AbilitySystemComponent;
}

void APTWBaseCharacter::HandleDeath(AActor* Attacker)
{
	if (!HasAuthority() || !AbilitySystemComponent) return;

	FGameplayEventData Payload;
	Payload.EventTag = FGameplayTag::RequestGameplayTag(FName("State.Status.Dead"));
	Payload.Instigator = Attacker;
	Payload.Target = this;
	UAbilitySystemBlueprintLibrary::SendGameplayEventToActor(this, Payload.EventTag, Payload);

	if (ReactorComponent)
	{
		ReactorComponent->ProcessDeath();
	}

	AbilitySystemComponent->AddLooseGameplayTag(DeadTag);

	if (OnCharacterDied.IsBound())
	{
		OnCharacterDied.Broadcast(this, Attacker);
	}

	if (APTWGameState* GS = GetWorld()->GetGameState<APTWGameState>())
	{
		AActor* MyPS = GetPlayerState();
		GS->Multicast_BroadcastKilllog(MyPS, Attacker);
	}
}

float APTWBaseCharacter::GetDamageMultiplier(const FName& BoneName) const
{
	if (BoneName == "head")
	{
		return 2.0f;
	}
	
	return 1.0f;
}

void APTWBaseCharacter::RemoveEffectWithTag(const FGameplayTag& TagToRemove)
{
	if (AbilitySystemComponent)
	{
		AbilitySystemComponent->RemoveActiveEffectsWithGrantedTags(FGameplayTagContainer(TagToRemove));
	}
}

void APTWBaseCharacter::ApplyGameplayEffectToSelf(TSubclassOf<class UGameplayEffect> EffectClass, float Level,
	FGameplayEffectContextHandle Context)
{
	if (AbilitySystemComponent && EffectClass)
	{
		FGameplayEffectSpecHandle SpecHandle = AbilitySystemComponent->MakeOutgoingSpec(EffectClass, Level, Context);
		if (SpecHandle.IsValid())
		{
			AbilitySystemComponent->ApplyGameplayEffectSpecToSelf(*SpecHandle.Data.Get());
		}
	}
}

void APTWBaseCharacter::ApplyGameplayEffectWithDuration(TSubclassOf<class UGameplayEffect> EffectClass, float Level,
	float Duration, FGameplayEffectContextHandle Context)
{
	if (AbilitySystemComponent && EffectClass)
	{
		FGameplayEffectSpecHandle SpecHandle = AbilitySystemComponent->MakeOutgoingSpec(EffectClass, Level, Context);
		if (SpecHandle.IsValid())
		{
			SpecHandle.Data->SetSetByCallerMagnitude(
				FGameplayTag::RequestGameplayTag(FName("Data.Duration")),
				Duration
				);
			
			AbilitySystemComponent->ApplyGameplayEffectSpecToSelf(*SpecHandle.Data.Get());
		}
	}
	
}


void APTWBaseCharacter::BeginPlay()
{
	Super::BeginPlay();
	
	if (HasAuthority())
	{
		AbilitySystemComponent->RemoveLooseGameplayTag(DeadTag);
	}
}

void APTWBaseCharacter::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	OnCharacterDied.RemoveAll(this);
	
	Super::EndPlay(EndPlayReason);
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
			if (AbilitySystemComponent->FindAbilitySpecFromClass(AbilityClass))
			{
				continue;
			}
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

	if (APTWPlayerState* PS = GetPlayerState<APTWPlayerState>())
	{
		PS->ApplyAdditionalAbilities();
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

	if (APTWPlayerState* PS = GetPlayerState<APTWPlayerState>())
	{
		PS->ApplyAdditionalEffects();
	}
}

