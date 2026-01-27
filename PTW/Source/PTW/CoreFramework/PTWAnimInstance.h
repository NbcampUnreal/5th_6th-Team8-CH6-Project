// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimInstance.h"
#include "GameplayTagContainer.h"
#include "PTWAnimInstance.generated.h"

class APTWPlayerCharacter;
class UAbilitySystemComponent;
class UCharacterMovementComponent;

USTRUCT(BlueprintType)
struct FPTWFootIKData
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly, Category = "IK")
	FVector FootLocationTarget = FVector::ZeroVector;

	UPROPERTY(BlueprintReadOnly, Category = "IK")
	FRotator FootRotationTarget = FRotator::ZeroRotator;

	UPROPERTY(BlueprintReadOnly, Category = "IK")
	float Alpha = 0.0f;
};

UCLASS()
class PTW_API UPTWAnimInstance : public UAnimInstance
{
	GENERATED_BODY()
	
public:
	virtual void NativeInitializeAnimation() override;
	virtual void NativeUpdateAnimation(float DeltaSeconds) override;
	virtual void NativeThreadSafeUpdateAnimation(float DeltaSeconds) override;

protected:
	void CalculateFootIK(FName SocketName, FPTWFootIKData& InFootData, float DeltaSeconds);

protected:
	UPROPERTY(BlueprintReadOnly, Category = "Character")
	TObjectPtr<APTWPlayerCharacter> Character;
	UPROPERTY(BlueprintReadOnly, Category = "Character")
	TObjectPtr<UCharacterMovementComponent> CharacterMovement;
	UPROPERTY(BlueprintReadOnly, Category = "Character")
	TObjectPtr<UAbilitySystemComponent> ASC;
	UPROPERTY(BlueprintReadOnly, Category = "Character")
	FGameplayTag WeaponStateTag;
	UPROPERTY(BlueprintReadOnly, Category = "Character")
	int32 WeaponPoseIndex = 0;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Config")
	TMap<FGameplayTag, int32> WeaponTagToPoseIndex;

	UPROPERTY(BlueprintReadOnly, Category = "Movement")
	FVector Velocity;
	UPROPERTY(BlueprintReadOnly, Category = "Movement")
	float GroundSpeed;
	UPROPERTY(BlueprintReadOnly, Category = "Movement")
	bool bShouldMove;
	UPROPERTY(BlueprintReadOnly, Category = "Movement")
	bool bIsFalling;
	UPROPERTY(BlueprintReadOnly, Category = "Movement")
	bool bIsCrouching;
	UPROPERTY(BlueprintReadOnly, Category = "Movement")
	bool bIsSprinting;
	UPROPERTY(BlueprintReadOnly, Category = "AimOffset")
	float AimPitch;
	UPROPERTY(BlueprintReadOnly, Category = "Movement")
	float LocomotionDirection;
	UPROPERTY(BlueprintReadOnly, Category = "IK|Hand")
	FTransform LeftHandIKTransform;
	UPROPERTY(BlueprintReadOnly, Category = "IK|Hand")
	float LeftHandIKAlpha;

	UPROPERTY(BlueprintReadOnly, Category = "IK|Foot")
	FPTWFootIKData FootData_L;
	UPROPERTY(BlueprintReadOnly, Category = "IK|Foot")
	FPTWFootIKData FootData_R;
	UPROPERTY(BlueprintReadOnly, Category = "IK|Foot")
	FVector PelvisOffset = FVector::ZeroVector;

	float IKTraceDistance = 55.0f;
	float IKInterpSpeed = 15.0f;
	float FootHeightOffset = 5.0f;
};
