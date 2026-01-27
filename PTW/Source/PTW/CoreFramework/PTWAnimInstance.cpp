// Fill out your copyright notice in the Description page of Project Settings.


#include "CoreFramework/PTWAnimInstance.h"
#include "CoreFramework/PTWPlayerCharacter.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "AbilitySystemComponent.h"
#include "AbilitySystemGlobals.h"
#include "Kismet/KismetMathLibrary.h"
#include "Inventory/PTWWeaponActor.h"
#include "Inventory/PTWInventoryComponent.h"
#include "Inventory/PTWItemInstance.h"
#include "Engine/Engine.h"
#include "Components/CapsuleComponent.h"


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

	//1인칭 및 3인칭 메시 HandIK 부착
	USkeletalMeshComponent* MyOwningMesh = GetOwningComponent();

	if (Character && MyOwningMesh)
	{
		UPTWInventoryComponent* Inventory = Character->GetInventoryComponent();

		if (Inventory)
		{
			UPTWItemInstance* CurrentItem = Inventory->GetCurrentWeaponInst();
			if (CurrentItem)
			{
				UMeshComponent* TargetWeaponMesh = nullptr;

				if (MyOwningMesh == Character->GetMesh1P())
				{
					if (CurrentItem->SpawnedWeapon1P)
					{
						TargetWeaponMesh = CurrentItem->SpawnedWeapon1P->GetStaticMeshComponent();
					}
				}
				else
				{
					if (CurrentItem->SpawnedWeapon3P)
					{
						TargetWeaponMesh = CurrentItem->SpawnedWeapon3P->GetStaticMeshComponent();
					}
				}

				if (TargetWeaponMesh)
				{
					FTransform SocketTransform = TargetWeaponMesh->GetSocketTransform(FName("LHIK"), RTS_World);
					FTransform MeshTransform = MyOwningMesh->GetComponentTransform();

					LeftHandIKTransform = SocketTransform.GetRelativeTransform(MeshTransform);
					LeftHandIKAlpha = 1.0f;
				}
				else
				{
					LeftHandIKAlpha = 0.0f;
				}
			}
			else
			{
				LeftHandIKAlpha = 0.0f;
			}
		}
	}
}

void UPTWAnimInstance::NativeThreadSafeUpdateAnimation(float DeltaSeconds)
{
	Super::NativeThreadSafeUpdateAnimation(DeltaSeconds);

	if (!Character) return;
	
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

	if (GetOwningComponent() == Character->GetMesh1P()) return;

	CalculateFootIK(FName("foot_l"), FootData_L, DeltaSeconds);
	CalculateFootIK(FName("foot_r"), FootData_R, DeltaSeconds);

	float HipTarget = FMath::Min(FootData_L.FootLocationTarget.Z, FootData_R.FootLocationTarget.Z);

	HipTarget = FMath::Min(0.0f, HipTarget);

	float CurrentHipZ = PelvisOffset.Z;
	float NewHipZ = FMath::FInterpTo(CurrentHipZ, HipTarget, DeltaSeconds, IKInterpSpeed);
	PelvisOffset = FVector(0.0f, 0.0f, NewHipZ);
}

void UPTWAnimInstance::CalculateFootIK(FName SocketName, FPTWFootIKData& OutFootData, float DeltaSeconds)
{
	USkeletalMeshComponent* Mesh = GetOwningComponent();
	if (!Mesh) return;

	FTransform SocketTransform = Mesh->GetSocketTransform(SocketName, RTS_Component);
	FVector SocketLoc = SocketTransform.GetLocation();

	FVector Start = FVector(SocketLoc.X, SocketLoc.Y, IKTraceDistance);
	FVector End = FVector(SocketLoc.X, SocketLoc.Y, -IKTraceDistance * 2.0f);

	FVector WorldStart = Mesh->GetComponentTransform().TransformPosition(Start);
	FVector WorldEnd = Mesh->GetComponentTransform().TransformPosition(End);

	FHitResult HitResult;
	FCollisionQueryParams Params;
	Params.AddIgnoredActor(Character);

	bool bHit = GetWorld()->LineTraceSingleByChannel(HitResult, WorldStart, WorldEnd, ECC_Visibility, Params);

	FVector TargetLoc = SocketLoc;
	FRotator TargetRot = FRotator::ZeroRotator;
	float TargetAlpha = 0.0f;

	if (bHit)
	{
		FVector HitLocInComp = Mesh->GetComponentTransform().InverseTransformPosition(HitResult.ImpactPoint);

		float FloorZ = HitLocInComp.Z + FootHeightOffset;

		if (SocketLoc.Z < FloorZ)
		{
			TargetLoc.Z = FloorZ;
			TargetAlpha = 1.0f;

			FVector ImpactNormal = HitResult.ImpactNormal;

			FVector NormalInComp = Mesh->GetComponentTransform().InverseTransformVector(ImpactNormal);

			float Pitch = -FMath::Atan2(NormalInComp.X, NormalInComp.Z) * (180.0f / PI);
			float Roll = FMath::Atan2(NormalInComp.Y, NormalInComp.Z) * (180.0f / PI);
			TargetRot = FRotator(Pitch, 0.0f, Roll);
		}
	}

	OutFootData.FootLocationTarget.X = SocketLoc.X;
	OutFootData.FootLocationTarget.Y = SocketLoc.Y;
	OutFootData.FootLocationTarget.Z = FMath::FInterpTo(OutFootData.FootLocationTarget.Z, TargetLoc.Z, DeltaSeconds, IKInterpSpeed);

	OutFootData.FootRotationTarget = FMath::RInterpTo(OutFootData.FootRotationTarget, TargetRot, DeltaSeconds, IKInterpSpeed);
	OutFootData.Alpha = FMath::FInterpTo(OutFootData.Alpha, TargetAlpha, DeltaSeconds, IKInterpSpeed);
}
