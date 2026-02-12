// Fill out your copyright notice in the Description page of Project Settings.


#include "CoreFramework/Character/AnimNotifies/PTWAnimNotify_Footstep.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/Character.h"
#include "PhysicalMaterials/PhysicalMaterial.h"
#include "CoreFramework/PTWPlayerCharacter.h"

UPTWAnimNotify_Footstep::UPTWAnimNotify_Footstep()
{
	NotifyColor = FColor(196, 142, 255, 255);
}

void UPTWAnimNotify_Footstep::Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, const FAnimNotifyEventReference& EventReference)
{
	Super::Notify(MeshComp, Animation, EventReference);

	if (!MeshComp || !MeshComp->GetOwner()) return;

	AActor* OwnerActor = MeshComp->GetOwner();
	APTWPlayerCharacter* PTWChar = Cast<APTWPlayerCharacter>(OwnerActor);

	if (PTWChar)
	{
		bool bIsLocallyControlled = PTWChar->IsLocallyControlled();

		if (bIsLocallyControlled)
		{
			if (MeshComp == PTWChar->GetMesh1P())
			{
				return;
			}
		}
		else
		{
			if (MeshComp != PTWChar->GetMesh3P())
			{
				return;
			}
		}
	}

	UWorld* World = MeshComp->GetWorld();
	if (!World) return;

	FVector FootLocation = MeshComp->GetSocketLocation(FootBoneName);
	FVector TraceEnd = FootLocation - FVector(0, 0, 50.0f);

	FHitResult HitResult;
	FCollisionQueryParams Params;
	Params.AddIgnoredActor(OwnerActor);
	Params.bReturnPhysicalMaterial = true;

	bool bHit = World->LineTraceSingleByChannel(HitResult, FootLocation, TraceEnd, ECC_Visibility, Params);

	USoundBase* SoundToPlay = DefaultFootstepSound;

	if (bHit && HitResult.PhysMaterial.IsValid())
	{
		EPhysicalSurface SurfaceType = HitResult.PhysMaterial->SurfaceType;

		//TODO : 머터리얼에 따라 소리 재생
	}

	if (SoundToPlay)
	{
		UGameplayStatics::PlaySoundAtLocation(
			World,
			SoundToPlay,
			FootLocation,
			VolumeMultiplier,
			PitchMultiplier,
			0.0f,
			NULL
		);
	}
}
