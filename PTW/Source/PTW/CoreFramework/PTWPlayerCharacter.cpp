// Fill out your copyright notice in the Description page of Project Settings.


#include "PTWPlayerCharacter.h"
#include "GameFramework/SpringArmComponent.h"
#include "Camera/CameraComponent.h"
#include "AbilitySystemComponent.h"
#include "PTWPlayerState.h"
#include "PTW/GAS/PTWAbilitySystemComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "PTWInputComponent.h"
#include "GAS/PTWGameplayAbility.h"
#include "Inventory/PTWInventoryComponent.h"
#include "Inventory/PTWWeaponActor.h"

APTWPlayerCharacter::APTWPlayerCharacter()
{
	PrimaryActorTick.bCanEverTick = true;

	bUseControllerRotationYaw = true;

	PlayerCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("PlayerCamera"));
	PlayerCamera->SetupAttachment(RootComponent);
	PlayerCamera->bUsePawnControlRotation = true;

	Mesh1P = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("CharacterMesh1P"));
	Mesh1P->SetupAttachment(PlayerCamera);

	GetCharacterMovement()->NavAgentProps.bCanCrouch = true;
	CrouchedEyeHeight = 40.0f;

	InventoryComponent = CreateDefaultSubobject<UPTWInventoryComponent>(TEXT("InventoryComponent"));
	InventoryComponent->SetIsReplicated(true);
}

void APTWPlayerCharacter::BeginPlay()
{
	Super::BeginPlay();

	if (APlayerController* PlayerController = Cast<APlayerController>(Controller))
	{
		if (UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(PlayerController->GetLocalPlayer()))
		{
			if (DefaultMappingContext)
			{
				Subsystem->AddMappingContext(DefaultMappingContext, 0);
			}
		}
	}
	if (Mesh1P)
	{
		Mesh1P->SetOnlyOwnerSee(true);
		Mesh1P->SetVisibility(true);
		Mesh1P->HideBoneByName(FName("head"), EPhysBodyOp::PBO_None);
	}
	
	if (GetWorld())
	{
		for (const auto& Pair : WeaponClasses)
		{
			FGameplayTag Tag = Pair.Key;
			TSubclassOf<APTWWeaponActor> ClassToSpawn = Pair.Value;

			if (ClassToSpawn)
			{
				FActorSpawnParameters Params;
				Params.Owner = this;

				APTWWeaponActor* Weapon1P = GetWorld()->SpawnActor<APTWWeaponActor>(ClassToSpawn, Params);
				APTWWeaponActor* Weapon3P = GetWorld()->SpawnActor<APTWWeaponActor>(ClassToSpawn, Params);

				if (Weapon1P && Weapon3P)
				{
					InventoryComponent->AddItem(ItemDef, Weapon1P);
					SpawnedWeapons.Add(Tag, Weapon1P);
					Weapon1P->AttachToComponent(GetMesh1P(), FAttachmentTransformRules::SnapToTargetNotIncludingScale, TEXT("WeaponSocket"));
					Weapon3P->AttachToComponent(GetMesh3P(), FAttachmentTransformRules::SnapToTargetNotIncludingScale, TEXT("WeaponSocket"));
					
					Weapon1P->SetupVisualPerspective(true);
					Weapon1P->SetupVisualPerspective(false);
				}
			}
		}
	}
	
}

void APTWPlayerCharacter::PossessedBy(AController* NewController)
{
	Super::PossessedBy(NewController);

	InitAbilityActorInfo();

	GiveDefaultAbilities();
	ApplyDefaultEffects();
}

void APTWPlayerCharacter::OnRep_PlayerState()
{
	Super::OnRep_PlayerState();

	InitAbilityActorInfo();
}

void APTWPlayerCharacter::InitAbilityActorInfo()
{
	Super::InitAbilityActorInfo();

	APTWPlayerState* PS = GetPlayerState<APTWPlayerState>();
	if (PS)
	{
		PS->GetAbilitySystemComponent()->InitAbilityActorInfo(PS, this);

		AbilitySystemComponent = PS->GetAbilitySystemComponent();
		AttributeSet = PS->GetAttributeSet();
	}
}

void APTWPlayerCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	if (UEnhancedInputComponent* EnhancedInputComponent = Cast<UEnhancedInputComponent>(PlayerInputComponent))
	{
		if (MoveAction)
		{
			EnhancedInputComponent->BindAction(MoveAction, ETriggerEvent::Triggered, this, &APTWPlayerCharacter::Move);
		}
		if (LookAction)
		{
			EnhancedInputComponent->BindAction(LookAction, ETriggerEvent::Triggered, this, &APTWPlayerCharacter::Look);
		}
		
		UPTWInputComponent* PTWInputComp = CastChecked<UPTWInputComponent>(PlayerInputComponent);

		TArray<uint32> BindHandles;
		PTWInputComp->BindAbilityActions(
			InputConfig,
			this,
			&ThisClass::Input_AbilityInputTagPressed,
			&ThisClass::Input_AbilityInputTagReleased,
			BindHandles
		);
	}
}

void APTWPlayerCharacter::Move(const FInputActionValue& Value)
{
	FVector2D MovementVector = Value.Get<FVector2D>();

	if (Controller != nullptr)
	{
		const FRotator Rotation = Controller->GetControlRotation();
		const FRotator YawRotation(0, Rotation.Yaw, 0);

		const FVector ForwardDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);

		const FVector RightDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);

		AddMovementInput(ForwardDirection, MovementVector.Y);
		AddMovementInput(RightDirection, MovementVector.X);
	}
}

void APTWPlayerCharacter::Look(const FInputActionValue& Value)
{
	FVector2D LookAxisVector = Value.Get<FVector2D>();

	if (Controller != nullptr)
	{
		AddControllerYawInput(LookAxisVector.X);
		AddControllerPitchInput(LookAxisVector.Y);
	}
}

void APTWPlayerCharacter::Input_AbilityInputTagPressed(FGameplayTag InputTag)
{
	if (UPTWAbilitySystemComponent* PTWASC = Cast<UPTWAbilitySystemComponent>(GetAbilitySystemComponent()))
	{
		PTWASC->AbilityInputTagPressed(InputTag);
	}
}

void APTWPlayerCharacter::Input_AbilityInputTagReleased(FGameplayTag InputTag)
{
	if (UPTWAbilitySystemComponent* PTWASC = Cast<UPTWAbilitySystemComponent>(GetAbilitySystemComponent()))
	{
		PTWASC->AbilityInputTagReleased(InputTag);
	}
}
