// Fill out your copyright notice in the Description page of Project Settings.


#include "PTWGA_Barrier.h"

#include "CoreFramework/PTWPlayerCharacter.h"
#include "Inventory/Item/BarrierActor.h"

void UPTWGA_Barrier::ApplyItemEffect()
{
	APTWPlayerCharacter* PC = Cast<APTWPlayerCharacter>(GetAvatarActorFromActorInfo());
	
	if (!PC) return;
	
	FActorSpawnParameters SpawnParams;
	SpawnParams.Owner = PC;
	SpawnParams.Instigator = PC;
	
	FRotator SpawnRotation = PC->GetActorRotation();
	
	GetWorld()->SpawnActor<ABarrierActor>(BarrierClass, SpawnLocation(), SpawnRotation, SpawnParams);
}

FVector UPTWGA_Barrier::SpawnLocation()
{
	APTWPlayerCharacter* PC = Cast<APTWPlayerCharacter>(GetAvatarActorFromActorInfo());
	
	FVector StartLocation = PC->GetActorLocation();
	FVector ForwardDirection = PC->GetActorForwardVector();
	
	float MaxSpawnDist = 200.f;
	float BarrierThickness = 50.f;
	
	FVector DesiredLocation = StartLocation + (ForwardDirection * MaxSpawnDist);
	FVector FinalSpawnLocation = DesiredLocation;
	
	FHitResult HitResult;
	FCollisionQueryParams CollisionParams;
	CollisionParams.AddIgnoredActor(PC);
	
	bool bIsHit = GetWorld()->LineTraceSingleByChannel(HitResult, StartLocation, DesiredLocation, ECC_Visibility, CollisionParams);
	
	if (bIsHit)
	{
		FinalSpawnLocation = HitResult.ImpactPoint - (ForwardDirection * BarrierThickness);
		float DistToHit = FVector::Dist(StartLocation, FinalSpawnLocation);
		
		if (DistToHit < 100.f) 
		{
			UE_LOG(LogTemp, Warning, TEXT("너무 가까움"));
		}
	}
	
	return FinalSpawnLocation;
}
