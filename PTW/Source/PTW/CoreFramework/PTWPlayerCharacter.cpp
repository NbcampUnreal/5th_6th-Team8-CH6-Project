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
#include "Inventory/PTWItemDefinition.h"
#include "System/PTWItemSpawnManager.h"
#include "Net/UnrealNetwork.h"

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

void APTWPlayerCharacter::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(APTWPlayerCharacter, CurrentWeaponTag);
	DOREPLIFETIME(APTWPlayerCharacter, CurrentWeapon);
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
	
	if (GetWorld() && HasAuthority())
	{
		UE_LOG(LogTemp, Log, TEXT("BeginPlay Weapon Setup for: %s"), *GetName());
		
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
				
				Weapon1P->bIsFirstPersonWeapon = true;
				Weapon3P->bIsFirstPersonWeapon = false;
				
				if (Weapon1P && Weapon3P)
				{
					InventoryComponent->AddItem(ItemDef, Weapon1P, Weapon3P);
					
					FWeaponPair Weaponpair;
					
					Weaponpair.Weapon1P = Weapon1P;
					Weaponpair.Weapon3P = Weapon3P;
					
					SpawnedWeapons.Add(Tag, Weaponpair);
					Weapon1P->AttachToComponent(GetMesh1P(), FAttachmentTransformRules::SnapToTargetNotIncludingScale, TEXT("WeaponSocket"));
					Weapon3P->AttachToComponent(GetMesh3P(), FAttachmentTransformRules::SnapToTargetNotIncludingScale, TEXT("WeaponSocket"));
					
					Weapon1P->ApplyVisualPerspective(); 
					Weapon3P->ApplyVisualPerspective();
					
					Weapon1P->SetActorHiddenInGame(true);
					Weapon3P->SetActorHiddenInGame(true);
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

	if (IsLocallyControlled())
	{
		InitAbilityActorInfo();
	}
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
	
		// [중요 디버깅 로그]
		UE_LOG(LogTemp, Warning, TEXT("[%s] InitAbility - PS: %s, Avatar: %s"), 
			HasAuthority() ? TEXT("SERVER") : TEXT("CLIENT"),
			*PS->GetName(), 
			*GetName());
	}
	else {
		UE_LOG(LogTemp, Error, TEXT("InitAbility Failed: PlayerState is NULL for %s"), *GetName());
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

void APTWPlayerCharacter::EquipWeaponByTag(FGameplayTag NewWeaponTag)
{
	if (!HasAuthority()) return;

	// 같은 무기면 해제 로직
	if (CurrentWeaponTag == NewWeaponTag)
	{
		if (CurrentWeapon)
		{
			if (FWeaponPair* CurrentPair = SpawnedWeapons.Find(CurrentWeaponTag))
			{
				if (CurrentPair->Weapon1P) CurrentPair->Weapon1P->SetActorHiddenInGame(true);
				if (CurrentPair->Weapon3P) CurrentPair->Weapon3P->SetActorHiddenInGame(true);
			}

			CurrentWeapon = nullptr;
		}

		CurrentWeaponTag = FGameplayTag::EmptyTag;
		UE_LOG(LogTemp, Log, TEXT("Weapon Unequipped (Toggle Off)"));
		return;
	}

	if (CurrentWeaponTag.IsValid())
	{
		if (FWeaponPair* OldPair = SpawnedWeapons.Find(CurrentWeaponTag))
		{
			if (OldPair->Weapon1P) OldPair->Weapon1P->SetActorHiddenInGame(true);
			if (OldPair->Weapon3P) OldPair->Weapon3P->SetActorHiddenInGame(true);
		}
	}

	if (FWeaponPair* FoundPair = SpawnedWeapons.Find(NewWeaponTag))
	{
		APTWWeaponActor* NewWeapon1P = FoundPair->Weapon1P;
		APTWWeaponActor* NewWeapon3P = FoundPair->Weapon3P;

		if (NewWeapon1P && NewWeapon3P)
		{
			NewWeapon1P->SetActorHiddenInGame(false);
			NewWeapon3P->SetActorHiddenInGame(false);

			CurrentWeapon = NewWeapon1P;
			CurrentWeaponTag = NewWeaponTag;

			UE_LOG(LogTemp, Log, TEXT("Weapon Equipped: %s"), *NewWeaponTag.ToString());
		}
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("Cannot find weapon with tag: %s"), *NewWeaponTag.ToString());
	}
}

void APTWPlayerCharacter::OnRep_CurrentWeapon(APTWWeaponActor* OldWeapon)
{
	if (OldWeapon)
	{
		OldWeapon->SetActorHiddenInGame(true);
	}

	if (CurrentWeapon)
	{
		CurrentWeapon->SetActorHiddenInGame(false);
		CurrentWeapon->ApplyVisualPerspective();
	}
}
