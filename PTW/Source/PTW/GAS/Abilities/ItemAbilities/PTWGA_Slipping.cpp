// Fill out your copyright notice in the Description page of Project Settings.


#include "PTWGA_Slipping.h"

#include "CoreFramework/PTWPlayerCharacter.h"
#include "GameFramework/CharacterMovementComponent.h"

class UCharacterMovementComponent;

void UPTWGA_Slipping::ActivateAbility(const FGameplayAbilitySpecHandle Handle,
                                      const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo,
                                      const FGameplayEventData* TriggerEventData)
{
	Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);
	
	if (!CommitAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo)) return;
    
	APTWPlayerCharacter* Character = Cast<APTWPlayerCharacter>(GetAvatarActorFromActorInfo());
	if (!Character || !Character->GetCharacterMovement()) return;

	UCharacterMovementComponent* MoveComp = Character->GetCharacterMovement();
	
	FVector LaunchDirection = Character->GetActorForwardVector();
	float LaunchStrength = 350.f;
	Character->LaunchCharacter(LaunchDirection * LaunchStrength, true, true);
	
	float OriginalBraking = MoveComp->BrakingDecelerationWalking;
	float OriginalFriction = MoveComp->GroundFriction;

	MoveComp->BrakingDecelerationWalking = 0.f; 
	MoveComp->GroundFriction = 0.05f;         
	
	FTimerHandle SlippingTimerHandle;
	GetWorld()->GetTimerManager().SetTimer(SlippingTimerHandle, [this, MoveComp, OriginalBraking, OriginalFriction]()
	{
		if (MoveComp)
		{
			MoveComp->BrakingDecelerationWalking = OriginalBraking;
			MoveComp->GroundFriction = OriginalFriction;
		}
		
		EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, false);
        
	}, 1.2f, false);
}

