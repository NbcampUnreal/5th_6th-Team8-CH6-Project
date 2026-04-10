#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "Animation/AnimInstance.h"
#include "PTWCitizenAnimInstance.generated.h"

class APTWBaseCitizen;
class UCharacterMovementComponent;
class UAbilitySystemComponent;

UCLASS()
class PTW_API UPTWCitizenAnimInstance : public UAnimInstance
{
	GENERATED_BODY()
	
	virtual void NativeInitializeAnimation() override;
	virtual void NativeUpdateAnimation(float DeltaSeconds) override;
	virtual void NativeThreadSafeUpdateAnimation(float DeltaSeconds) override;

protected:
	UPROPERTY(BlueprintReadOnly, Category = "Character")
	TObjectPtr<APTWBaseCitizen> Character;
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
	UPROPERTY(BlueprintReadOnly, Category = "IK")
	FTransform LeftHandIKTransform;
	UPROPERTY(BlueprintReadOnly, Category = "IK")
	float LeftHandIKAlpha;
	UPROPERTY(BlueprintReadOnly, Category = "Movement")
	float LocomotionStartDirection;
};
