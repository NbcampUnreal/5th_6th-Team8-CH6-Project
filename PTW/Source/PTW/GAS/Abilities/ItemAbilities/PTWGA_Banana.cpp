// Fill out your copyright notice in the Description page of Project Settings.


#include "PTWGA_Banana.h"

#include "CoreFramework/PTWPlayerCharacter.h"
#include "GameFramework/ProjectileMovementComponent.h"
#include "Inventory/Item/BananaItemActor.h"

void UPTWGA_Banana::ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo,
                                    const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData)
{
	Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);
}

void UPTWGA_Banana::InitializeVariable()
{
	Super::InitializeVariable();
}

void UPTWGA_Banana::ApplyItemEffect()
{
	if (!CommitAbilityCooldown(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true))
	{
		return;
	}
	
	APTWPlayerCharacter* PC = Cast<APTWPlayerCharacter>(GetAvatarActorFromActorInfo());
	if (!PC) return;
	
	FVector CameraLocation;
	FRotator CameraRotation;
	PC->GetController()->GetPlayerViewPoint(CameraLocation, CameraRotation);
	
	FVector TraceEnd = CameraLocation + (CameraRotation.Vector() * 10000.0f);
	FHitResult ScreenHit;
	FCollisionQueryParams Params;
	Params.AddIgnoredActor(PC);
	
	
	GetWorld()->LineTraceSingleByChannel(ScreenHit, CameraLocation, TraceEnd, ECC_Visibility, Params);
	
	FVector TargetLocation = ScreenHit.bBlockingHit ? ScreenHit.ImpactPoint : TraceEnd;
	FVector SpawnLocation = PC->GetActorLocation() + PC->GetActorForwardVector() * 50.f + PC->GetActorUpVector() * 50.f;
	FVector LaunchDirection = (TargetLocation - SpawnLocation).GetSafeNormal();
	LaunchDirection.Z += 0.2f;
	LaunchDirection.Normalize();
	
	FActorSpawnParameters SpawnParams;
	SpawnParams.Owner = PC;
	SpawnParams.Instigator = PC;

	ABananaItemActor* SpawnedBanana = GetWorld()->SpawnActor<ABananaItemActor>(BananaActor, SpawnLocation, LaunchDirection.Rotation(), SpawnParams);

	if (SpawnedBanana && SpawnedBanana->GetProjectileMovementComponent())
	{
		float LaunchSpeed = 1500.f; 
		SpawnedBanana->GetProjectileMovementComponent()->Velocity = LaunchDirection * LaunchSpeed;
	}
	
	
}
