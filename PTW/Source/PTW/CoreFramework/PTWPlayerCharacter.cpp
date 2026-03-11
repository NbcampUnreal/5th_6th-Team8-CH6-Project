// Fill out your copyright notice in the Description page of Project Settings.


#include "PTWPlayerCharacter.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "Net/UnrealNetwork.h"
#include "GameplayTagContainer.h"
#include "Camera/CameraComponent.h"
#include "AbilitySystemComponent.h"
#include "PTW.h"
#include "Components/WidgetComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Components/SceneCaptureComponent2D.h"

#include "PTWPlayerState.h"
#include "PTWInputComponent.h"
#include "PTWPlayerController.h"
#include "Components/SphereComponent.h"
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
#include "MiniGame/PTWMiniGameMode.h"
#include "Net/VoiceConfig.h"

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
	
	PushCollision = CreateDefaultSubobject<USphereComponent>(TEXT("SphereComponent"));
	PushCollision->SetupAttachment(GetMesh(), TEXT("Hand_R_Socket")); 
	PushCollision->SetSphereRadius(30.0f);
	
	PushCollision->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	PushCollision->SetCollisionResponseToAllChannels(ECR_Ignore);
	PushCollision->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);
	
	VOIPTalkerComponent = CreateDefaultSubobject<UVOIPTalker>(TEXT("VOIPTalker"));

	/* USceneCaptureComponent2D */
	TargetPOVSource = CreateDefaultSubobject<USceneCaptureComponent2D>(TEXT("TargetPOVSource"));
	if (PlayerCamera)
	{
		TargetPOVSource->SetupAttachment(PlayerCamera); // 카메라에 부착
	}
	// 프리미티브만 렌더링하여 성능 최적화
	TargetPOVSource->PrimitiveRenderMode = ESceneCapturePrimitiveRenderMode::PRM_RenderScenePrimitives;
	TargetPOVSource->bCaptureEveryFrame = false; // 수동 캡처(Timer) 사용
	TargetPOVSource->bCaptureOnMovement = false;
	// 가벼운 LDR 사용
	TargetPOVSource->CaptureSource = ESceneCaptureSource::SCS_FinalColorLDR;
}

void APTWPlayerCharacter::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(ThisClass, bIsStealth);
	DOREPLIFETIME(ThisClass, AimPitch);
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

	InitCharacterState();
	RegisterGameplayTagEvents();
}

void APTWPlayerCharacter::OnRep_PlayerState()
{
	Super::OnRep_PlayerState();

	InitCharacterState();
	RegisterGameplayTagEvents();
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

	if (AbilitySystemComponent)
	{
		AbilitySystemComponent->ClearActorInfo();
	}

	if (APTWPlayerState* PS = GetPlayerState<APTWPlayerState>())
	{
		PS->OnPlayerDataUpdated.RemoveDynamic(this, &APTWPlayerCharacter::OnPlayerDataLoaded);
	}

	Super::EndPlay(EndPlayReason);
}

void APTWPlayerCharacter::HandleDeath(AActor* Attacker)
{
	if (!HasAuthority() || !AbilitySystemComponent) return;
	
	Super::HandleDeath(Attacker);
	
	if(APTWPlayerController* PTWPC= GetController<APTWPlayerController>())
	{
		PTWPC->StartSpectating();
	}
}

void APTWPlayerCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	
	if (!IsLocallyControlled())
	{
		FRotator NewRotation = PlayerCamera->GetRelativeRotation();
		NewRotation.Pitch = AimPitch;
		PlayerCamera->SetRelativeRotation(NewRotation);
	}
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

	if (APlayerController* PC = Cast<APlayerController>(GetController()))
	{
		if (UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(PC->GetLocalPlayer()))
		{
			if (DefaultMappingContext)
			{
				Subsystem->AddMappingContext(DefaultMappingContext, 0);
			}
		}
	}
	
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
		if (EquipFirstWeaponAction && EquipSecondWeaponAction)
		{
			EnhancedInputComponent->BindAction(EquipFirstWeaponAction, ETriggerEvent::Started, this, &APTWPlayerCharacter::EquipFirstWeapon);
			EnhancedInputComponent->BindAction(EquipSecondWeaponAction, ETriggerEvent::Started, this, &APTWPlayerCharacter::EquipSecondWeapon);
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
		
		AimPitch = GetControlRotation().Pitch;
		ServerRPCUpdateAimPitch(AimPitch);
	}
}

void APTWPlayerCharacter::EquipFirstWeapon(const FInputActionValue& Value)
{
	if (IsLocallyControlled())
	{
		ServerRPCEquipWeapon(0);
	}
}

void APTWPlayerCharacter::EquipSecondWeapon(const FInputActionValue& Value)
{
	if (IsLocallyControlled())
	{
		ServerRPCEquipWeapon(1);
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
	// 1. 실행 주체 확인 로그
	FString NetRole = HasAuthority() ? TEXT("Server") : TEXT("Client");
	FString LocalStatus = IsLocallyControlled() ? TEXT("Local") : TEXT("Remote");

	APTWPlayerState* PS = GetPlayerState<APTWPlayerState>();

	// 2. PlayerState 복제 대기 로그
	if (!PS)
	{
		UE_LOG(LogTemp, Warning, TEXT("[InitChar][%s][%s] %s: PlayerState 대기 중..."), *NetRole, *LocalStatus, *GetName());
		StartInitTimer();
		return;
	}

	// 3. 중복 초기화 방지
	if (bIsAbilitiesInitialized)
	{
		GetWorldTimerManager().ClearTimer(InitTimerHandle);
		return;
	}

	// 4. GAS 초기화 시도 및 결과 확인
	InitAbilityActorInfo();

	UAbilitySystemComponent* ASC = GetAbilitySystemComponent();
	if (!ASC)
	{
		UE_LOG(LogTemp, Error, TEXT("[InitChar][%s][%s] %s: ASC가 여전히 Null입니다! 재시도 예약."), *NetRole, *LocalStatus, *GetName());
		StartInitTimer();
		return;
	}

	// 5. 서버 전용 로직 (태그 및 아이템)
	if (HasAuthority())
	{
		if (ASC->HasMatchingGameplayTag(GameplayTags::State::Status_Dead))
		{
			ASC->RemoveLooseGameplayTag(GameplayTags::State::Status_Dead);
			UE_LOG(LogTemp, Log, TEXT("[InitChar][Server] %s: 죽음 태그를 정리했습니다."), *PS->GetPlayerName());
		}

		FGameplayTag EquipTag = FGameplayTag::RequestGameplayTag(FName("Weapon.State.Equip"));
		if (ASC->HasMatchingGameplayTag(EquipTag))
		{
			ASC->SetLooseGameplayTagCount(EquipTag, 0);
			ASC->RemoveActiveEffectsWithTags(FGameplayTagContainer(EquipTag));
			UE_LOG(LogTemp, Log, TEXT("[InitChar][Server] %s: 장착 태그를 강제 초기화했습니다."), *PS->GetPlayerName());
		}

		if (!bHasGivenStartupItems)
		{
			FPTWPlayerData CurrentData = PS->GetPlayerData();
			if (CurrentData.InventoryItemIDs.Num() > 0)
			{
				UE_LOG(LogTemp, Log, TEXT("[InitChar][Server] %s: 기존 데이터 로드 성공 (아이템 %d개)"), *PS->GetPlayerName(), CurrentData.InventoryItemIDs.Num());
				OnPlayerDataLoaded(CurrentData);
			}
			else
			{
				UE_LOG(LogTemp, Log, TEXT("[InitChar][Server] %s: 초기 데이터 없음. 델리게이트 바인딩."), *PS->GetPlayerName());
				PS->OnPlayerDataUpdated.RemoveDynamic(this, &APTWPlayerCharacter::OnPlayerDataLoaded);
				PS->OnPlayerDataUpdated.AddDynamic(this, &APTWPlayerCharacter::OnPlayerDataLoaded);
			}
		}

		GiveDefaultAbilities();
		ApplyDefaultEffects();
	}

	// 6. 공통 컴포넌트 등록
	UpdateNameTagText();

	if (VOIPTalkerComponent)
	{
		VOIPTalkerComponent->RegisterWithPlayerState(PS);
	}

	// 7. 최종 완료 처리
	bIsAbilitiesInitialized = true;
	GetWorldTimerManager().ClearTimer(InitTimerHandle);

	UE_LOG(LogTemp, Log, TEXT("[InitChar][%s][%s] %s: 초기화 최종 완료!"), *NetRole, *LocalStatus, *PS->GetPlayerName());
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
		
		AbilitySystemComponent->RegisterGameplayTagEvent(GameplayTags::State::Charge, EGameplayTagEventType::AnyCountChange)
		.AddUObject(this, &APTWPlayerCharacter::OnMovelimit);
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

void APTWPlayerCharacter::OnMovelimit(const FGameplayTag Tag, int32 NewCount)
{
	APTWPlayerController* PC = Cast<APTWPlayerController>(Controller);
	if (!PC) return;
	
	if (NewCount > 0)
	{
		PC->SetIgnoreMoveInput(true);
		GetCharacterMovement()->StopMovementImmediately();
	}
	else
	{
		PC->ResetIgnoreMoveInput();
	}
}

void APTWPlayerCharacter::OnRep_StealthMode()
{
}

void APTWPlayerCharacter::StartInitTimer()
{
	if (!GetWorldTimerManager().IsTimerActive(InitTimerHandle))
	{
		GetWorldTimerManager().SetTimer(InitTimerHandle, this, &APTWPlayerCharacter::InitCharacterState, 0.2f, true);
	}
}

void APTWPlayerCharacter::ServerRPCUpdateAimPitch_Implementation(float NewAimPitch)
{
	AimPitch = NewAimPitch;
}

void APTWPlayerCharacter::SetStealthMode(bool bSetStealthMode)
{
	bIsStealth = bSetStealthMode;
	OnRep_StealthMode();
}

void APTWPlayerCharacter::ServerRPCUseActiveItem_Implementation()
{
	if (InventoryComponent)
	{
		InventoryComponent->UseActiveItem();
	}
}

void APTWPlayerCharacter::ServerRPCEquipWeapon_Implementation(int32 SelectIndex)
{
	if (InventoryComponent)
	{
		InventoryComponent->EquipWeapon(SelectIndex);
	}
}


void APTWPlayerCharacter::OnPlayerDataLoaded(const FPTWPlayerData& NewData)
{
	if (bHasGivenStartupItems || NewData.InventoryItemIDs.Num() == 0)
	{
		return;
	}

	APTWPlayerState* PS = GetPlayerState<APTWPlayerState>();
	if (!PS) return;

	if (UPTWItemSpawnManager* SpawnSys = GetWorld()->GetSubsystem<UPTWItemSpawnManager>())
	{
		if (APTWMiniGameMode* MiniGameMode = Cast<APTWMiniGameMode>(GetWorld()->GetAuthGameMode()))
		{
			if (!MiniGameMode->PlayerDeadCheck(GetController()))
			{
				SpawnSys->SpawnAndGiveItems(PS);
			}
			else
			{
				FItemArrayWrapper ItemArrayWrapper = MiniGameMode->GetOldPlayerItems(GetController());
				SpawnSys->AddRestartPlayerItems(ItemArrayWrapper.Items, this);
			}
		}
		
		bHasGivenStartupItems = true;
		PS->OnPlayerDataUpdated.RemoveDynamic(this, &APTWPlayerCharacter::OnPlayerDataLoaded);;
	}
}
