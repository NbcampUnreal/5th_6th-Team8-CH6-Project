#include "PTWCitizenAnimInstance.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "AbilitySystemComponent.h"
#include "AbilitySystemGlobals.h"
#include "Kismet/KismetMathLibrary.h"
#include "MiniGame/Character/AI/PTWBaseCitizen.h"

void UPTWCitizenAnimInstance::NativeInitializeAnimation()
{
	Super::NativeInitializeAnimation();

	Character = Cast<APTWBaseCitizen>(TryGetPawnOwner());
	if (Character)
	{
		CharacterMovement = Character->GetCharacterMovement();

		ASC = UAbilitySystemGlobals::GetAbilitySystemComponentFromActor(Character);
	}
}

void UPTWCitizenAnimInstance::NativeUpdateAnimation(float DeltaSeconds)
{
	Super::NativeUpdateAnimation(DeltaSeconds);

	if (!Character)
	{
		if (Character)
		{
			CharacterMovement = Character->GetCharacterMovement();
			ASC = UAbilitySystemGlobals::GetAbilitySystemComponentFromActor(Character);
		}
		return;
	}

	//이동속도 변수 업데이트
	Velocity = CharacterMovement->Velocity;
	GroundSpeed = Velocity.Size2D();

	bShouldMove = (GroundSpeed > 3.0f) && (CharacterMovement->GetCurrentAcceleration() != FVector::ZeroVector);

	bIsFalling = CharacterMovement->IsFalling();

	bIsCrouching = Character->bIsCrouched;

	if (GroundSpeed > 3.0f)
	{
		LocomotionDirection = CalculateDirection(Velocity, Character->GetActorRotation());
	}
	else
	{
		LocomotionDirection = 0.0f;
	}
	if (GroundSpeed < 10.0f && CharacterMovement->GetCurrentAcceleration().Size2D() > 0.0f)
	{
		LocomotionStartDirection = CalculateDirection(CharacterMovement->GetCurrentAcceleration(), Character->GetActorRotation());
	}
	if (GroundSpeed > 10.0f && CharacterMovement->GetCurrentAcceleration().Size2D() == 0.0f)
	{
		LocomotionStartDirection = CalculateDirection(Velocity, Character->GetActorRotation());
	}

	FRotator AimRotation = Character->GetBaseAimRotation();

	FRotator DeltaRot = AimRotation - Character->GetActorRotation();
	AimPitch = UKismetMathLibrary::NormalizeAxis(DeltaRot.Pitch);

	if (ASC)
	{
		FGameplayTag SprintTag = FGameplayTag::RequestGameplayTag(TEXT("Input.Action.Sprint"));
		bIsSprinting = ASC->HasMatchingGameplayTag(SprintTag);
	}
}

void UPTWCitizenAnimInstance::NativeThreadSafeUpdateAnimation(float DeltaSeconds)
{
	Super::NativeThreadSafeUpdateAnimation(DeltaSeconds);
}
