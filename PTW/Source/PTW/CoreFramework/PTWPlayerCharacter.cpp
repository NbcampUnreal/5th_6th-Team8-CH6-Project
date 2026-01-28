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
#include "PTWPlayerController.h"
#include "GAS/PTWGameplayAbility.h"
#include "Inventory/PTWInventoryComponent.h"
#include "Inventory/PTWWeaponActor.h"
#include "Inventory/PTWItemDefinition.h"
#include "System/PTWItemSpawnManager.h"
#include "Net/UnrealNetwork.h"
#include "Components/WidgetComponent.h" // PlayerNameTag
#include "UI/CharacterUI/PTWPlayerName.h" // PlayerNameTag
#include "CoreFramework/PTWPlayerState.h"
#include "PTW/Inventory/PTWWeaponData.h"
#include "GameplayTagContainer.h"

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

	/* PlayerNameTag */
	NameTagWidget = CreateDefaultSubobject<UWidgetComponent>(TEXT("NameTagWidget"));
	NameTagWidget->SetupAttachment(GetMesh()); // 메시에 부착
	NameTagWidget->SetRelativeLocation(FVector(0.f, 0.f, 200.f)); // 머리 위 적절한 높이
	NameTagWidget->SetWidgetSpace(EWidgetSpace::Screen); // 항상 화면을 향하도록 설정
	NameTagWidget->SetDrawAtDesiredSize(true);
}

void APTWPlayerCharacter::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(APTWPlayerCharacter, CurrentWeaponTag);
	DOREPLIFETIME(APTWPlayerCharacter, CurrentWeapon);
}

void APTWPlayerCharacter::HandleDeath(AActor* Attacker)
{
	Super::HandleDeath(Attacker);
	
	APTWPlayerController* PC = GetController<APTWPlayerController>();
	if (!PC)
	{
		return;
	}
	
	if (HasAuthority())
	{
		// TODO: 임시 관전 전환 로직
		PC->StartSpectating();
	}
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
}

void APTWPlayerCharacter::PossessedBy(AController* NewController)
{
	Super::PossessedBy(NewController);

	InitAbilityActorInfo();
	
	GiveDefaultAbilities();
	ApplyDefaultEffects();
	UpdateNameTagText(); // PlayerNameTag
}

void APTWPlayerCharacter::OnRep_PlayerState()
{
	Super::OnRep_PlayerState();

	UpdateNameTagText(); // PlayerNameTag

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

void APTWPlayerCharacter::AttachWeaponToSocket(APTWWeaponActor* NewWeapon1P, APTWWeaponActor* NewWeapon3P, FGameplayTag WeaponTag)
{
	if (!NewWeapon1P || !NewWeapon3P) return;

	FWeaponPair Weaponpair;
	Weaponpair.Weapon1P = NewWeapon1P;
	Weaponpair.Weapon3P = NewWeapon3P;
	SpawnedWeapons.Add(WeaponTag, Weaponpair);

	NewWeapon1P->AttachToComponent(GetMesh1P(), FAttachmentTransformRules::SnapToTargetNotIncludingScale, TEXT("WeaponSocket"));
	NewWeapon3P->AttachToComponent(GetMesh3P(), FAttachmentTransformRules::SnapToTargetNotIncludingScale, TEXT("WeaponSocket"));

	NewWeapon1P->ApplyVisualPerspective();
	NewWeapon3P->ApplyVisualPerspective();

	NewWeapon1P->SetActorHiddenInGame(true);
	NewWeapon3P->SetActorHiddenInGame(true);

	NewWeapon1P->SetActorEnableCollision(false);
	NewWeapon3P->SetActorEnableCollision(false);
}

UWidgetComponent* APTWPlayerCharacter::GetNameTagWidget() const
{
	return NameTagWidget;
}

void APTWPlayerCharacter::UpdateNameTagText()
{
	if (!NameTagWidget) return;

	// PlayerState에서 이름 가져오기
	APTWPlayerState* PS = GetPlayerState<APTWPlayerState>();
	UPTWPlayerName* NameWidget = Cast<UPTWPlayerName>(NameTagWidget->GetUserWidgetObject());

	if (!PS || !NameWidget)
	{
		GetWorldTimerManager().SetTimer(NameTagRetryTimer, this, &APTWPlayerCharacter::UpdateNameTagText, 0.2f, false);
		return;
	}
	GetWorldTimerManager().ClearTimer(NameTagRetryTimer);

	// 이름 설정 (1순위 : 플레이어데이터의 닉네임, 2순위 : 스팀아이디닉네임)
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
	
	// UI 반영
	NameWidget->SetPlayerName(Name);
}

void APTWPlayerCharacter::ApplyRecoil()
{
	if (!CurrentWeapon) return;
	const UPTWWeaponData* Data = CurrentWeapon->GetWeaponData();
	if (!Data) return;

	FGameplayTag FireTag = FGameplayTag::RequestGameplayTag(FName("Weapon.Anim.Fire"));
	if (Data->AnimMap.Contains(FireTag))
	{
		UAnimMontage* Montage = *Data->AnimMap.Find(FireTag);
		if (Montage)
		{
			UAnimInstance* AnimInstance = (IsLocallyControlled() && Mesh1P) ? Mesh1P->GetAnimInstance() : GetMesh()->GetAnimInstance();
			if (AnimInstance)
			{
				AnimInstance->Montage_Play(Montage, 1.0f);
			}
		}
	}
}
