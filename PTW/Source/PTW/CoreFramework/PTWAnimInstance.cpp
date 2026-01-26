// Fill out your copyright notice in the Description page of Project Settings.


#include "CoreFramework/PTWAnimInstance.h"
#include "CoreFramework/PTWPlayerCharacter.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "AbilitySystemComponent.h"
#include "AbilitySystemGlobals.h"
#include "Kismet/KismetMathLibrary.h"
#include "Inventory/PTWWeaponActor.h"

void UPTWAnimInstance::NativeInitializeAnimation()
{
	Super::NativeInitializeAnimation();

	Character = Cast<APTWPlayerCharacter>(TryGetPawnOwner());
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
		Character = Cast<APTWPlayerCharacter>(TryGetPawnOwner());
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

	if (GroundSpeed > 3.0f)
	{
		LocomotionDirection = CalculateDirection(Velocity, Character->GetActorRotation());
	}
	else
	{
		LocomotionDirection = 0.0f;
	}

	FRotator AimRotation = Character->GetBaseAimRotation();

	FRotator DeltaRot = AimRotation - Character->GetActorRotation();
	AimPitch = UKismetMathLibrary::NormalizeAxis(DeltaRot.Pitch);

	if (ASC)
	{
		FGameplayTag SprintTag = FGameplayTag::RequestGameplayTag(TEXT("Input.Action.Sprint"));
		bIsSprinting = ASC->HasMatchingGameplayTag(SprintTag);
	}

	USkeletalMeshComponent* MyOwningMesh = GetOwningComponent();
	APTWWeaponActor* Weapon = Character->GetCurrentWeapon();

	if (IsValid(Weapon) && MyOwningMesh)
	{
		UMeshComponent* TargetWeaponMesh = nullptr;

		auto* PlayerChar = Cast<APTWPlayerCharacter>(Character);

		if (PlayerChar && MyOwningMesh == PlayerChar->GetMesh1P())
		{
			TargetWeaponMesh = Weapon->GetStaticMeshComponent();
		}
		else
		{
			TargetWeaponMesh = Weapon->GetStaticMeshComponent();
		}

		if (TargetWeaponMesh)
		{
			FTransform SocketTransform = TargetWeaponMesh->GetSocketTransform(FName("LHIK"), RTS_World);

			FTransform MeshTransform = MyOwningMesh->GetComponentTransform();

			LeftHandIKTransform = SocketTransform.GetRelativeTransform(MeshTransform);
			LeftHandIKAlpha = 1.0f;
		}
	}
	else
	{
		LeftHandIKAlpha = 0.0f;
	}
}

void UPTWAnimInstance::NativeThreadSafeUpdateAnimation(float DeltaSeconds)
{
	Super::NativeThreadSafeUpdateAnimation(DeltaSeconds);

	if (Character)
	{
		FGameplayTag CurrentTag = Character->CurrentWeaponTag;

		if (const int32* FoundIndex = WeaponTagToPoseIndex.Find(CurrentTag))
		{
			WeaponPoseIndex = *FoundIndex;
		}
		else
		{
			WeaponPoseIndex = 0;
		}
	}
}
