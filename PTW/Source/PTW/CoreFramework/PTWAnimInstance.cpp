// Fill out your copyright notice in the Description page of Project Settings.


#include "CoreFramework/PTWAnimInstance.h"
#include "CoreFramework/PTWBaseCharacter.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "AbilitySystemComponent.h"
#include "AbilitySystemGlobals.h"

void UPTWAnimInstance::NativeInitializeAnimation()
{
	Super::NativeInitializeAnimation();

	Character = Cast<APTWBaseCharacter>(TryGetPawnOwner());
	if (Character)
	{
		CharacterMovement = Character->GetCharacterMovement();

		ASC = UAbilitySystemGlobals::GetAbilitySystemComponentFromActor(Character);
	}
}

void UPTWAnimInstance::NativeUpdateAnimation(float DeltaSeconds)
{
	Super::NativeUpdateAnimation(DeltaSeconds);

	if (!Character)
	{
		Character = Cast<APTWBaseCharacter>(TryGetPawnOwner());
		if (Character)
		{
			CharacterMovement = Character->GetCharacterMovement();
			ASC = UAbilitySystemGlobals::GetAbilitySystemComponentFromActor(Character);
		}
		return;
	}

	Velocity = CharacterMovement->Velocity;
	GroundSpeed = Velocity.Size2D();

	bShouldMove = (GroundSpeed > 3.0f) && (CharacterMovement->GetCurrentAcceleration() != FVector::ZeroVector);

	bIsFalling = CharacterMovement->IsFalling();

	bIsCrouching = Character->bIsCrouched;

	if (ASC)
	{
		FGameplayTag SprintTag = FGameplayTag::RequestGameplayTag(TEXT("Input.Action.Sprint"));
		bIsSprinting = ASC->HasMatchingGameplayTag(SprintTag);
	}
}
