// Fill out your copyright notice in the Description page of Project Settings.


#include "Gameplay/Actor/PTWResultCharacter.h"
#include "Components/SkeletalMeshComponent.h"

APTWResultCharacter::APTWResultCharacter()
{
	PrimaryActorTick.bCanEverTick = true;
	bReplicates = true;

	MeshComp = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("MeshComp"));
	RootComponent = MeshComp;

	MeshComp->SetCollisionProfileName(TEXT("NoCollision"));
	MeshComp->SetCastShadow(true);
}

void APTWResultCharacter::InitializeResult(bool bIsWinner)
{
	if (HasAuthority())
	{
		Multicast_SetResultState(bIsWinner);
	}
}

void APTWResultCharacter::Multicast_SetResultState_Implementation(bool bIsWinner)
{
	UAnimMontage* MontageToPlay = bIsWinner ? WinMontage : LoseMontage;

	if (MontageToPlay && MeshComp->GetAnimInstance())
	{
		MeshComp->GetAnimInstance()->Montage_Play(MontageToPlay);
	}
}
