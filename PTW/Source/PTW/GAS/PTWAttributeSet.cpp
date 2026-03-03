// Fill out your copyright notice in the Description page of Project Settings.

#include "PTW/GAS/PTWAttributeSet.h"

#include "Net/UnrealNetwork.h"
#include "GameplayEffectExtension.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "PTW/CoreFramework/PTWPlayerCharacter.h"
#include "CoreFramework/PTWPlayerController.h"
#include "CoreFramework/Character/Component/PTWReactorComponent.h"

UPTWAttributeSet::UPTWAttributeSet()
{
	InitHealth(100.0f);
	InitMaxHealth(100.0f);
	InitMoveSpeed(500.0f);
	InitJumpZVelocity(420.0f);
}

void UPTWAttributeSet::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME_CONDITION_NOTIFY(UPTWAttributeSet, Health, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UPTWAttributeSet, MaxHealth, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UPTWAttributeSet, MoveSpeed, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UPTWAttributeSet, JumpZVelocity, COND_None, REPNOTIFY_Always);
}


void UPTWAttributeSet::PreAttributeChange(const FGameplayAttribute& Attribute, float& NewValue)
{
	Super::PreAttributeChange(Attribute, NewValue);

	if (Attribute == GetHealthAttribute())
	{
		NewValue = FMath::Clamp(NewValue, 0.0f, GetMaxHealth());
	}
	if (Attribute == GetMoveSpeedAttribute())
	{
		NewValue = FMath::Max(NewValue, 0.0f);
	}
	else if (Attribute == GetJumpZVelocityAttribute())
	{
		NewValue = FMath::Max(NewValue, 0.0f);
	}
}

void UPTWAttributeSet::PostGameplayEffectExecute(const FGameplayEffectModCallbackData& Data)
{
	Super::PostGameplayEffectExecute(Data);

	FGameplayEffectContextHandle Context = Data.EffectSpec.GetContext();
	UAbilitySystemComponent* Source = Context.GetOriginalInstigatorAbilitySystemComponent();
	AActor* SourceActor = Context.GetInstigator();
	if (Data.EvaluatedData.Attribute == GetHealthAttribute())
	{
		if (Data.EvaluatedData.Magnitude < 0.f)
		{
			HandleDamage(Data);
		}

		SetHealth(FMath::Clamp(GetHealth(), 0.0f, GetMaxHealth()));

		if (GetHealth() <= 0.0f)
		{
			AActor* TargetActor = Data.Target.GetAvatarActor();
			APTWBaseCharacter* TargetCharacter = Cast<APTWBaseCharacter>(TargetActor);

			if (TargetCharacter && !TargetCharacter->IsDead())
			{
				TargetCharacter->HandleDeath(SourceActor);
			}
		}
		if (GetHealth() > 0.0f)
		{
			APTWBaseCharacter* TargetChar = Cast<APTWBaseCharacter>(Data.Target.GetAvatarActor());
			if (TargetChar && !TargetChar->IsDead())
			{
				const FHitResult* Hit = Data.EffectSpec.GetContext().GetHitResult();
				FVector ImpactPoint = Hit ? (FVector)Hit->ImpactPoint : TargetChar->GetActorLocation();

				TargetChar->GetReactorComponent()->Multicast_PlayHitReact(ImpactPoint);
			}
		}

	}

	AActor* TargetActor = nullptr;
	ACharacter* TargetCharacter = nullptr;
	if (Data.Target.AbilityActorInfo.IsValid() && Data.Target.AbilityActorInfo->AvatarActor.IsValid())
	{
		TargetActor = Data.Target.AbilityActorInfo->AvatarActor.Get();
		TargetCharacter = Cast<ACharacter>(TargetActor);
	}

	if (TargetCharacter && TargetCharacter->GetCharacterMovement())
	{
		if (Data.EvaluatedData.Attribute == GetMoveSpeedAttribute())
		{
			TargetCharacter->GetCharacterMovement()->MaxWalkSpeed = GetMoveSpeed();
		}
		else if (Data.EvaluatedData.Attribute == GetJumpZVelocityAttribute())
		{
			TargetCharacter->GetCharacterMovement()->JumpZVelocity = GetJumpZVelocity();
		}
	}

}

void UPTWAttributeSet::PostAttributeChange(const FGameplayAttribute& Attribute, float OldValue, float NewValue)
{
	Super::PostAttributeChange(Attribute, OldValue, NewValue);

	if (Attribute == GetMoveSpeedAttribute())
	{
		UAbilitySystemComponent* ASC = GetOwningAbilitySystemComponent();
		if (ASC && ASC->GetAvatarActor())
		{
			ACharacter* Character = Cast<ACharacter>(ASC->GetAvatarActor());
			if (Character && Character->GetCharacterMovement())
			{
				Character->GetCharacterMovement()->MaxWalkSpeed = NewValue;
			}
		}
	}
	if (Attribute == GetJumpZVelocityAttribute())
	{
		UAbilitySystemComponent* ASC = GetOwningAbilitySystemComponent();
		if (ASC && ASC->GetAvatarActor())
		{
			ACharacter* Character = Cast<ACharacter>(ASC->GetAvatarActor());
			if (Character && Character->GetCharacterMovement())
			{
				Character->GetCharacterMovement()->JumpZVelocity = NewValue;
			}
		}
	}
}

void UPTWAttributeSet::OnRep_Health(const FGameplayAttributeData& OldHealth) { GAMEPLAYATTRIBUTE_REPNOTIFY(UPTWAttributeSet, Health, OldHealth); }
void UPTWAttributeSet::OnRep_MaxHealth(const FGameplayAttributeData& OldMaxHealth) { GAMEPLAYATTRIBUTE_REPNOTIFY(UPTWAttributeSet, MaxHealth, OldMaxHealth); }

void UPTWAttributeSet::OnRep_MoveSpeed(const FGameplayAttributeData& OldMoveSpeed)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UPTWAttributeSet, MoveSpeed, OldMoveSpeed);
	
	if (UAbilitySystemComponent* ASC = GetOwningAbilitySystemComponent())
	{
		if (ACharacter* Character = Cast<ACharacter>(ASC->GetAvatarActor()))
		{
			if (UCharacterMovementComponent* MoveComp = Character->GetCharacterMovement())
			{
				MoveComp->MaxWalkSpeed = GetMoveSpeed();
			}
		}
	}
}

void UPTWAttributeSet::OnRep_JumpZVelocity(const FGameplayAttributeData& OldJumpZVelocity) { GAMEPLAYATTRIBUTE_REPNOTIFY(UPTWAttributeSet, JumpZVelocity, OldJumpZVelocity); }

void UPTWAttributeSet::HandleDamage(const FGameplayEffectModCallbackData& Data)
{
	AActor* TargetActor = Data.Target.GetAvatarActor();
	if (!TargetActor) return;

	APawn* TargetPawn = Cast<APawn>(TargetActor);
	if (!TargetPawn) return;

	FVector DamageCauserLocation = FVector::ZeroVector;

	const FGameplayEffectContextHandle& Context =
		Data.EffectSpec.GetContext();

	if (Context.GetHitResult())
	{
		DamageCauserLocation =
			Context.GetHitResult()->TraceStart;
	}
	else if (AActor* Instigator = Context.GetInstigator())
	{
		DamageCauserLocation =
			Instigator->GetActorLocation();
	}

	// PlayerController로 전달
	if (APTWPlayerController* PC = Cast<APTWPlayerController>(TargetPawn->GetController()))
	{
		PC->ClientRPC_ShowDamageIndicator(DamageCauserLocation);
	}
}
