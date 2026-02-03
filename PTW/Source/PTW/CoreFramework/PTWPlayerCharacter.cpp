// Fill out your copyright notice in the Description page of Project Settings.


#include "PTWPlayerCharacter.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "Net/UnrealNetwork.h"
#include "GameplayTagContainer.h"
#include "Camera/CameraComponent.h"
#include "AbilitySystemComponent.h"
#include "Components/WidgetComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "GameFramework/CharacterMovementComponent.h"

#include "PTWPlayerState.h"
#include "PTWInputComponent.h"
#include "PTWPlayerController.h"
#include "GAS/PTWGameplayAbility.h"
#include "System/PTWItemSpawnManager.h"
#include "PTW/GAS/PTWAbilitySystemComponent.h"
#include "Inventory/PTWInventoryComponent.h"
#include "Inventory/PTWItemDefinition.h"
#include "UI/CharacterUI/PTWPlayerName.h"
#include "CoreFramework/Character/Component/PTWWeaponComponent.h"
#include "CoreFramework/Character/Component/PTWReactorComponent.h"

APTWPlayerCharacter::APTWPlayerCharacter()
{
	PrimaryActorTick.bCanEverTick = true;

	bUseControllerRotationYaw = true;

	PlayerCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("PlayerCamera"));
	PlayerCamera->SetupAttachment(RootComponent);
	PlayerCamera->bUsePawnControlRotation = true;

	Mesh1P = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("CharacterMesh1P"));
	Mesh1P->SetupAttachment(PlayerCamera);
	Mesh1P->SetHiddenInGame(true);

	GetCharacterMovement()->NavAgentProps.bCanCrouch = true;
	CrouchedEyeHeight = 40.0f;

	InventoryComponent = CreateDefaultSubobject<UPTWInventoryComponent>(TEXT("InventoryComponent"));
	InventoryComponent->SetIsReplicated(true);

	/* PlayerNameTag */
	NameTagWidget = CreateDefaultSubobject<UWidgetComponent>(TEXT("NameTagWidget"));
	NameTagWidget->SetupAttachment(GetMesh());
	NameTagWidget->SetRelativeLocation(FVector(0.f, 0.f, 200.f));
	NameTagWidget->SetWidgetSpace(EWidgetSpace::Screen);
	NameTagWidget->SetDrawAtDesiredSize(true);

	GetCharacterMovement()->bOrientRotationToMovement = false;
	GetCharacterMovement()->bUseControllerDesiredRotation = true;
	GetCharacterMovement()->RotationRate = FRotator(0.0f, 540.0f, 0.0f);

	WeaponComponent = CreateDefaultSubobject<UPTWWeaponComponent>(TEXT("WeaponComponent"));
	WeaponComponent->SetIsReplicated(true);
}

void APTWPlayerCharacter::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

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
		Mesh1P->SetHiddenInGame(false);
		Mesh1P->SetVisibility(true);
		Mesh1P->HideBoneByName(FName("head"), EPhysBodyOp::PBO_None);
	}
}

void APTWPlayerCharacter::PossessedBy(AController* NewController)
{
	Super::PossessedBy(NewController);

	if (AbilitySystemComponent)
	{
		InitAbilityActorInfo();

		if (HasAuthority())
		{
			AbilitySystemComponent->RemoveLooseGameplayTag(DeadTag);
		}
		FGameplayTag EquipTag = FGameplayTag::RequestGameplayTag(FName("Weapon.State.Equip"));

		if (AbilitySystemComponent->HasMatchingGameplayTag(EquipTag))
		{
			AbilitySystemComponent->SetLooseGameplayTagCount(EquipTag, 0);

			AbilitySystemComponent->RemoveActiveEffectsWithTags(FGameplayTagContainer(EquipTag));

			UE_LOG(LogTemp, Warning, TEXT("PossessedBy: Force Removed Equip Tag"));
		}
	}

	GiveDefaultAbilities();
	ApplyDefaultEffects();
	UpdateNameTagText();
}

void APTWPlayerCharacter::OnRep_PlayerState()
{
	Super::OnRep_PlayerState();

	InitAbilityActorInfo();

	UpdateNameTagText(); // PlayerNameTag

	if (IsLocallyControlled())
	{
		if (InventoryComponent)
		{
			InventoryComponent->SetCurrentWeaponInst(nullptr);
		}
		if (AbilitySystemComponent)
		{
			FGameplayTag EquipTag = FGameplayTag::RequestGameplayTag(FName("Weapon.State.Equip"));
			AbilitySystemComponent->SetLooseGameplayTagCount(EquipTag, 0);
			if (HasAuthority())
			{
				AbilitySystemComponent->RemoveLooseGameplayTag(DeadTag);
			}
		}
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
	
		UE_LOG(LogTemp, Warning, TEXT("[%s] InitAbility - PS: %s, Avatar: %s"), 
			HasAuthority() ? TEXT("SERVER") : TEXT("CLIENT"),
			*PS->GetName(), 
			*GetName());
	}
	else 
	{
		UE_LOG(LogTemp, Error, TEXT("InitAbility Failed: PlayerState is NULL for %s"), *GetName());
	}
}

void APTWPlayerCharacter::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	if (GetWorldTimerManager().IsTimerActive(NameTagRetryTimer))
	{
		GetWorldTimerManager().ClearTimer(NameTagRetryTimer);
	}

	Super::EndPlay(EndPlayReason);
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
		if (EquipWeaponAction)
		{
			EnhancedInputComponent->BindAction(EquipWeaponAction, ETriggerEvent::Started, this, &APTWPlayerCharacter::EquipWeapon);
		}
		if (UseActiveItemAction)
		{
			EnhancedInputComponent->BindAction(UseActiveItemAction, ETriggerEvent::Started, this, &APTWPlayerCharacter::UseActiveItem);
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

void APTWPlayerCharacter::EquipWeapon(const FInputActionValue& Value)
{
	ServerRPCEquipWeapon();
}

void APTWPlayerCharacter::UseActiveItem(const FInputActionValue& Value)
{
	ServerRPCUseActiveItem();
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

UWidgetComponent* APTWPlayerCharacter::GetNameTagWidget() const
{
	return NameTagWidget;
}

void APTWPlayerCharacter::UpdateNameTagText()
{
	if (!NameTagWidget) return;

	APTWPlayerState* PS = GetPlayerState<APTWPlayerState>();
	UPTWPlayerName* NameWidget = Cast<UPTWPlayerName>(NameTagWidget->GetUserWidgetObject());

	if (!PS || !NameWidget)
	{
		GetWorldTimerManager().SetTimer(NameTagRetryTimer, this, &APTWPlayerCharacter::UpdateNameTagText, 0.2f, false);
		return;
	}
	GetWorldTimerManager().ClearTimer(NameTagRetryTimer);

	FString Name;
	const FPTWPlayerData& PD = PS->GetPlayerData();
	if (!PD.PlayerName.IsEmpty())
	{
		Name = PD.PlayerName;
	}
	else
	{
		Name = PS->GetPlayerName();
	}

	NameWidget->SetPlayerName(Name);
}

void APTWPlayerCharacter::ServerRPCUseActiveItem_Implementation()
{
	if (InventoryComponent)
	{
		InventoryComponent->UseActiveItem();
	}
}

void APTWPlayerCharacter::ServerRPCEquipWeapon_Implementation()
{
	if (InventoryComponent)
	{
		InventoryComponent->EquipWeapon(0);
	}
}
