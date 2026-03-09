// Fill out your copyright notice in the Description page of Project Settings.


#include "PTWCARCitizen.h"
#include "PTWGameplayTag/GameplayTags.h"
#include "AbilitySystemComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GAS/PTWAttributeSet.h"
#include "MiniGame/PTWMiniGameMode.h"

APTWCARCitizen::APTWCARCitizen()
{
	PrimaryActorTick.bCanEverTick = false;
	
	AutoPossessAI = EAutoPossessAI::PlacedInWorldOrSpawned;
	AttributeSet = CreateDefaultSubobject<UPTWAttributeSet>(TEXT("AttributeSet"));
	
	bUseControllerRotationPitch = false;
	bUseControllerRotationYaw = false;
	bUseControllerRotationRoll = false;
	
	if (GetCharacterMovement())
	{
		GetCharacterMovement()->bOrientRotationToMovement = true; 
		GetCharacterMovement()->RotationRate = FRotator(0.0f, 500.0f, 0.0f); 
	}
}

void APTWCARCitizen::BeginPlay()
{
	Super::BeginPlay();
	
	if (HasAuthority())
	{
		InitAbilityActorInfo();
		
		if (APTWMiniGameMode* MiniGameMode = Cast<APTWMiniGameMode>(GetWorld()->GetAuthGameMode()))
		{
			OnCharacterDied.AddUniqueDynamic(MiniGameMode, &APTWMiniGameMode::HandlePlayerDeath);
		}
	}
}

void APTWCARCitizen::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	if (HasAuthority())
	{
		if (APTWMiniGameMode* MiniGameMode = Cast<APTWMiniGameMode>(GetWorld()->GetAuthGameMode()))
		{
			OnCharacterDied.RemoveDynamic(MiniGameMode, &APTWMiniGameMode::HandlePlayerDeath);
		}
	}
	
	Super::EndPlay(EndPlayReason);
}

void APTWCARCitizen::InitAbilityActorInfo()
{
	Super::InitAbilityActorInfo();
	
	check(AbilitySystemComponent);
	AbilitySystemComponent->InitAbilityActorInfo(this, this);
	
	AbilitySystemComponent->AddLooseGameplayTag(GameplayTags::Role::Citizen, 1, EGameplayTagReplicationState::TagOnly);
	UE_LOG(LogTemp, Log, TEXT("[%s] InitAbilityActorInfo Success - Owner: %s"),
		HasAuthority() ? TEXT("SERVER") : TEXT("CLIENT"), *GetName());
}

void APTWCARCitizen::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

void APTWCARCitizen::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);
}

