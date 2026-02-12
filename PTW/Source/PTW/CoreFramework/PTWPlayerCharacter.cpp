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
#include "CoreFramework/Character/Component/PTWInteractComponent.h"
#include "PTWGameplayTag/GameplayTags.h"
#include "Kismet/GameplayStatics.h"

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

	InteractComponent = CreateDefaultSubobject<UPTWInteractComponent>(TEXT("InteractComponent"));
	InteractComponent->SetIsReplicated(true);
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
	
	RegisterGameplayTagEvents();
}

void APTWPlayerCharacter::PossessedBy(AController* NewController)
{
	Super::PossessedBy(NewController);

	InitCharacterState();
}

void APTWPlayerCharacter::OnRep_PlayerState()
{
	Super::OnRep_PlayerState();

	InitCharacterState();
}

void APTWPlayerCharacter::OnPlayerStateChanged(APlayerState* NewPlayerState, APlayerState* OldPlayerState)
{
	Super::OnPlayerStateChanged(NewPlayerState, OldPlayerState);

	if (NewPlayerState)
	{
		InitCharacterState();
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

		UE_LOG(LogTemp, Log, TEXT("[%s] InitAbilityActorInfo Success - Owner: %s"),
			HasAuthority() ? TEXT("SERVER") : TEXT("CLIENT"), *GetName());
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

void APTWPlayerCharacter::Landed(const FHitResult& Hit)
{
	Super::Landed(Hit);

	if (LandSound)
	{
		UGameplayStatics::PlaySoundAtLocation(this, LandSound, GetActorLocation());
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
			EnhancedInputComponent->BindAction(MoveAction, ETriggerEvent::Triggered, this, &APTWPlayerCharacter::OnInputTriggered);
			EnhancedInputComponent->BindAction(MoveAction, ETriggerEvent::Completed, this, &APTWPlayerCharacter::OnInputCompleted);
		}
		if (LookAction)
		{
			EnhancedInputComponent->BindAction(LookAction, ETriggerEvent::Triggered, this, &APTWPlayerCharacter::Look);
			EnhancedInputComponent->BindAction(LookAction, ETriggerEvent::Triggered, this, &APTWPlayerCharacter::OnInputTriggered);
			EnhancedInputComponent->BindAction(LookAction, ETriggerEvent::Completed, this, &APTWPlayerCharacter::OnInputCompleted);
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

	if (APTWPlayerController* PC = Cast<APTWPlayerController>(GetController()))
	{
		AddControllerYawInput(LookAxisVector.X * PC->CurrentMouseSensitivity);
		AddControllerPitchInput(LookAxisVector.Y * PC->CurrentMouseSensitivity);
	}
}

void APTWPlayerCharacter::EquipWeapon(const FInputActionValue& Value)
{
	if (IsLocallyControlled())
	{

		ServerRPCEquipWeapon();
	}
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

void APTWPlayerCharacter::InitCharacterState()
{
	APTWPlayerState* PS = GetPlayerState<APTWPlayerState>();
	if (!PS || bIsAbilitiesInitialized) return;

	InitAbilityActorInfo();

	if (HasAuthority())
	{
		if (AbilitySystemComponent)
		{
			AbilitySystemComponent->RemoveLooseGameplayTag(GameplayTags::State::Status_Dead);

			FGameplayTag EquipTag = FGameplayTag::RequestGameplayTag(FName("Weapon.State.Equip"));
			if (AbilitySystemComponent->HasMatchingGameplayTag(EquipTag))
			{
				AbilitySystemComponent->SetLooseGameplayTagCount(EquipTag, 0);
				AbilitySystemComponent->RemoveActiveEffectsWithTags(FGameplayTagContainer(EquipTag));
				UE_LOG(LogTemp, Warning, TEXT("InitCharacterState: Force Removed Equip Tag"));
			}
		}

		if (UPTWItemSpawnManager* SpawnSys = GetWorld()->GetSubsystem<UPTWItemSpawnManager>())
		{
			SpawnSys->SpawnAndGiveItems(PS);
			UE_LOG(LogTemp, Log, TEXT("[Init] Items Spawned for Player: %s"), *PS->GetPlayerName());
		}
	}

	GiveDefaultAbilities();
	ApplyDefaultEffects();
	UpdateNameTagText();

	bIsAbilitiesInitialized = true;
}

void APTWPlayerCharacter::OnInputTriggered()
{
	GetWorldTimerManager().ClearTimer(IdleCheckTimerHandle);

	if (bIsIdleState)
	{
		SetIdleState(false);
	}
}

void APTWPlayerCharacter::OnInputCompleted()
{
	if (!GetWorldTimerManager().IsTimerActive(IdleCheckTimerHandle))
	{
		GetWorldTimerManager().SetTimer(IdleCheckTimerHandle, this, &APTWPlayerCharacter::CheckIdleCondition, 0.1f, true);
	}
}

void APTWPlayerCharacter::CheckIdleCondition()
{
	bool bIsStationary = GetVelocity().SizeSquared() < 10.0f;
	bool bIsNotFalling = !GetCharacterMovement()->IsFalling();

	if (bIsStationary && bIsNotFalling)
	{
		SetIdleState(true);

		GetWorldTimerManager().ClearTimer(IdleCheckTimerHandle);
	}
}

void APTWPlayerCharacter::SetIdleState(bool bNewState)
{
	if (bIsIdleState == bNewState) return;

	bIsIdleState = bNewState;

	UAbilitySystemComponent* ASC = GetAbilitySystemComponent();
	if (!ASC) return;

	FGameplayTag IdleTag = FGameplayTag::RequestGameplayTag(FName("State.Idle"));

	if (bIsIdleState)
	{
		ASC->AddLooseGameplayTag(IdleTag);
	}
	else
	{
		// 태그 제거
		ASC->RemoveLooseGameplayTag(IdleTag);
	}
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

void APTWPlayerCharacter::RegisterGameplayTagEvents()
{
	if (AbilitySystemComponent)
	{
		AbilitySystemComponent->RegisterGameplayTagEvent(GameplayTags::State::Stasis, EGameplayTagEventType::AnyCountChange)
		.AddUObject(this, &APTWPlayerCharacter::OnStasisTagChanged);
	}
}

void APTWPlayerCharacter::OnStasisTagChanged(const FGameplayTag Tag, int32 NewCount)
{
	APTWPlayerController* PC = Cast<APTWPlayerController>(Controller);
	if (!PC) return;
	
	if (NewCount > 0)
	{
		PC->SetIgnoreLookInput(true);
		PC->SetIgnoreMoveInput(true);
		GetCharacterMovement()->StopMovementImmediately();
	}
	else
	{
		PC->ResetIgnoreLookInput();
		PC->ResetIgnoreMoveInput();
	}
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
