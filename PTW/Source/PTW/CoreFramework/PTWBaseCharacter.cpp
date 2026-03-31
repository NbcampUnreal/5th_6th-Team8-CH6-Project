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
#include "MiniGame/GameMode/PTWBombMiniGameMode.h"
#include "PTWGameplayTag/GameplayTags.h"

APTWBaseCharacter::APTWBaseCharacter()
{
	PrimaryActorTick.bCanEverTick = false;

	bAlwaysRelevant = true;

	if (USkeletalMeshComponent* CharacterMesh = GetMesh())
	{
		CharacterMesh->SetCullDistance(0.0f);
		CharacterMesh->BoundsScale = 2.0f;
		CharacterMesh->VisibilityBasedAnimTickOption = EVisibilityBasedAnimTickOption::AlwaysTickPoseAndRefreshBones;
	}

	ReactorComponent = CreateDefaultSubobject<UPTWReactorComponent>(TEXT("ReactorComponent"));
}

bool APTWBaseCharacter::IsDead() const
{
	return IsValid(AbilitySystemComponent) && AbilitySystemComponent->HasMatchingGameplayTag(GameplayTags::State::Status_Dead);
}

UAbilitySystemComponent* APTWBaseCharacter::GetAbilitySystemComponent() const
{
	return AbilitySystemComponent;
}

void APTWBaseCharacter::HandleDeath(AActor* Attacker)
{
	if (!HasAuthority() || !AbilitySystemComponent) return;

	FGameplayEventData Payload;
	Payload.EventTag = GameplayTags::State::Status_Dead;
	Payload.Instigator = Attacker;
	Payload.Target = this;
	UAbilitySystemBlueprintLibrary::SendGameplayEventToActor(this, Payload.EventTag, Payload);

	if (ReactorComponent)
	{
		ReactorComponent->ProcessDeath();
	}

	AbilitySystemComponent->AddLooseGameplayTag(GameplayTags::State::Status_Dead);

	if (OnCharacterDied.IsBound())
	{
		OnCharacterDied.Broadcast(this, Attacker);
	}

	if (APTWGameState* GS = GetWorld()->GetGameState<APTWGameState>())
	{
		AActor* DeadActor = nullptr;
		if (GetPlayerState())
		{
			DeadActor = GetPlayerState();
		}
		else
		{
			DeadActor = this;
		}
		
		if (GetWorld() && GetWorld()->GetAuthGameMode<APTWBombMiniGameMode>())
		{
			return;
		}
		
		GS->Multicast_BroadcastKilllog(DeadActor, Attacker);
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
	
	if (HasAuthority() && IsValid(AbilitySystemComponent) && AbilitySystemComponent->HasMatchingGameplayTag(GameplayTags::State::Status_Dead))
	{
		AbilitySystemComponent->RemoveLooseGameplayTag(GameplayTags::State::Status_Dead);
	}
}

void APTWBaseCharacter::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	OnCharacterDied.Clear();
	
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
			FGameplayAbilitySpec* ExistingSpec = AbilitySystemComponent->FindAbilitySpecFromClass(AbilityClass);
			if (ExistingSpec)
			{
				AbilitySystemComponent->ClearAbility(ExistingSpec->Handle);
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

