// Fill out your copyright notice in the Description page of Project Settings.


#include "MiniGame/Character/RedLight/PTWRedLightCharacter.h"
#include "MiniGame/GameMode/PTWRedLightGameMode.h"
#include "MiniGame/Widget/RedLight/PTWRedLightMark.h"
#include "Camera/CameraComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Components/WidgetComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Net/UnrealNetwork.h"
#include "DrawDebugHelpers.h"
#include "Components/SpotLightComponent.h"

APTWRedLightCharacter::APTWRedLightCharacter()
{
	FirstPersonCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("FirstPersonCamera"));
	FirstPersonCamera->SetupAttachment(GetMesh(), TEXT("head"));
	FirstPersonCamera->bUsePawnControlRotation = true;

	if (GetMesh())
	{
		GetMesh()->bOwnerNoSee = true;
	}

	if (GetCharacterMovement())
	{
		GetCharacterMovement()->MaxWalkSpeed = 0.f;
		GetCharacterMovement()->DisableMovement();
	}

	LeftEyeLight = CreateDefaultSubobject<USpotLightComponent>(TEXT("LeftEyeLight"));
	LeftEyeLight->SetupAttachment(GetMesh(), TEXT("head"));
	RightEyeLight = CreateDefaultSubobject<USpotLightComponent>(TEXT("RightEyeLight"));
	RightEyeLight->SetupAttachment(GetMesh(), TEXT("head"));
}

void APTWRedLightCharacter::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(APTWRedLightCharacter, bIsRedLight);
}

void APTWRedLightCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

}

void APTWRedLightCharacter::UpdateEyeLights()
{
	if (LeftEyeLight) LeftEyeLight->SetVisibility(bIsRedLight);
	if (RightEyeLight) RightEyeLight->SetVisibility(bIsRedLight);
}

void APTWRedLightCharacter::ToggleLight()
{
	if (IsLocallyControlled())
	{
		Server_SetLightState(!bIsRedLight);
	}
}

void APTWRedLightCharacter::Server_SetLightState_Implementation(bool bNewState)
{
	bIsRedLight = bNewState;

	if (APTWRedLightGameMode* GM = Cast<APTWRedLightGameMode>(GetWorld()->GetAuthGameMode()))
	{
		GM->OnRedLightStateChanged(bIsRedLight, this);
	}
}

void APTWRedLightCharacter::OnRep_IsRedLight()
{
	UpdateEyeLights();
}

void APTWRedLightCharacter::Multicast_SpottedPlayer_Implementation(ACharacter* CaughtPlayer)
{
	if (!CaughtPlayer || !MarkWidgetClass) return;

	UWidgetComponent* MarkComp = NewObject<UWidgetComponent>(CaughtPlayer);
	MarkComp->RegisterComponent();
	MarkComp->AttachToComponent(CaughtPlayer->GetRootComponent(), FAttachmentTransformRules::KeepRelativeTransform);

	MarkComp->SetRelativeLocation(FVector(0.f, 0.f, 100.f));
	MarkComp->SetWidgetSpace(EWidgetSpace::Screen);
	MarkComp->SetWidgetClass(MarkWidgetClass);
	MarkComp->SetDrawSize(FVector2D(150.f, 150.f));
	MarkComp->InitWidget();

	if (UPTWRedLightMark* MarkWidget = Cast<UPTWRedLightMark>(MarkComp->GetWidget()))
	{
		MarkWidget->PlaySpottedAnimation();
	}
}



