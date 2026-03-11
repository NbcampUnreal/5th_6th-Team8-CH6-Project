// Fill out your copyright notice in the Description page of Project Settings.


#include "CoreFramework/PTWSpectatorPawn.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "PTWBaseCharacter.h"
#include "PTWPlayerController.h"
#include "Components/SphereComponent.h"
#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"
#include "GameFramework/PlayerState.h"
#include "GameFramework/SpringArmComponent.h"

APTWSpectatorPawn::APTWSpectatorPawn()
{
	PrimaryActorTick.bCanEverTick = true;
	
	SpringArmComponent = CreateDefaultSubobject<USpringArmComponent>("SpringArmComponent");
	SpringArmComponent->SetupAttachment(GetCollisionComponent());
	
	CameraComponent = CreateDefaultSubobject<UCameraComponent>("CameraComponent");
	CameraComponent->SetupAttachment(SpringArmComponent, USpringArmComponent::SocketName);
	
	bUseControllerRotationPitch = false;
	bUseControllerRotationYaw = false;
	bUseControllerRotationRoll = false;
	
	SpringArmComponent->bUsePawnControlRotation = true; 
	SpringArmComponent->bInheritPitch = true;
	SpringArmComponent->bInheritYaw = true;
	SpringArmComponent->bInheritRoll = false;
	
	SpringArmComponent->ProbeSize = 12.0f;
	
	MaxZoom = 500.0f;
	MinZoom = 100.0f;
	ZoomStep = 100.0f;
	
	Starting3PZoomDistance = FMath::Lerp(MinZoom, MaxZoom, 0.5f);
	Current3PZoomDistance = Starting3PZoomDistance;
	
	bIsFreeCamera = true;
	CurrentZoomDistance = MinZoom;
	
	bIsFirstPerson = true;
	
	bAddDefaultMovementBindings = false;
	
	SpringArmComponent->bEnableCameraLag = true;
	SpringArmComponent->CameraLagSpeed = 10.0f;

	SpringArmComponent->bEnableCameraRotationLag = true;
	SpringArmComponent->CameraRotationLagSpeed = 15.0f;
}

void APTWSpectatorPawn::SetViewTarget(bool bIsSameViewTarget)
{
	APlayerController* PC = GetController<APlayerController>();
	if (!IsValid(PC)) return;
	
	if (bIsFirstPerson)
	{
		CurrentZoomDistance = 0.0f;
		SpringArmComponent->SocketOffset = FVector::ZeroVector;
		
		AActor* NewViewTarget = CurrentViewCharacter;
		if (!IsValid(NewViewTarget))
		{
			NewViewTarget = PC;
		}
		
		if (bIsSameViewTarget)
		{
			PC->SetViewTarget(NewViewTarget);
		}
		else
		{
			PC->SetViewTargetWithBlend(NewViewTarget, 1.0f, VTBlend_Cubic);
		}
	}
	else
	{
		CurrentZoomDistance = Current3PZoomDistance;
		SpringArmComponent->SocketOffset = FVector(0.0f, 0.0f, 40.0f);
		PC->SetViewTarget(this);
	}
}

void APTWSpectatorPawn::Move(const FInputActionValue& Value)
{
	if (GetAttachParentActor()) return;
	
	FVector2D MovementVector = Value.Get<FVector2D>();

	if (IsValid(Controller))
	{
		const FRotator Rotation = Controller->GetControlRotation();
		const FRotator YawRotation(0, Rotation.Yaw, 0);

		const FVector ForwardDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);

		const FVector RightDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);

		AddMovementInput(ForwardDirection, MovementVector.Y);
		AddMovementInput(RightDirection, MovementVector.X);
	}
}

void APTWSpectatorPawn::Look(const FInputActionValue& Value)
{
	FVector2D LookAxisVector = Value.Get<FVector2D>();
	
	if (APTWPlayerController* PC = GetController<APTWPlayerController>())
	{
		AddControllerYawInput(LookAxisVector.X * PC->CurrentMouseSensitivity);
		AddControllerPitchInput(LookAxisVector.Y * PC->CurrentMouseSensitivity);
	}
}

void APTWSpectatorPawn::Zoom(const FInputActionValue& Value)
{
	if (bIsFirstPerson) return;
	
	float InputValue = Value.Get<float>();
	if (FMath::IsNearlyZero(InputValue)) return;
	
	CurrentZoomDistance -= (InputValue * ZoomStep);
	CurrentZoomDistance = FMath::Clamp(CurrentZoomDistance, MinZoom, MaxZoom);
}

void APTWSpectatorPawn::SpectateNextPlayer()
{
	APawn* NewTargetView = nullptr;
	if (FindNextSpectatorTarget(NewTargetView))
	{
		SetSpectatorTarget(NewTargetView);
	}
}

void APTWSpectatorPawn::BeginSpectate()
{
	GetWorldTimerManager().SetTimer(SpectateTimer, this, &ThisClass::SpectateNextPlayer, 3.0f, false);
}

bool APTWSpectatorPawn::FindNextSpectatorTarget(APawn*& NewViewTarget)
{
	// APlayerState* TempPS = GetPlayerState();
	// UE_LOG(LogTemp, Warning, TEXT("GetPlayerState = %s"), TempPS ? *TempPS->GetName() : nullptr);
	// UE_LOG(LogTemp, Warning, TEXT("Controller = %s"), *Controller->GetName());
	// UE_LOG(LogTemp, Warning, TEXT("Controller->PS = %s"), *GetController<APlayerController>()->PlayerState->GetName());
	// APlayerController* TEMPPC =  GetController<APlayerController>();
	// UE_LOG(LogTemp, Warning, TEXT("Controller->GetPawn() = %s"), TEMPPC ? *TEMPPC->GetName() : nullptr);
	// UE_LOG(LogTemp, Warning, TEXT("Controller->PS->GetPawn() = %s"), TEMPPC->PlayerState->GetPawn() ? *TEMPPC->PlayerState->GetPawn()->GetName() : nullptr);
	
	UWorld* World = GetWorld();
	if (!IsValid(World)) return false;
	
	APlayerController* PC = GetController<APlayerController>();
	if (!IsValid(PC)) return false;

	APlayerState* PS = PC->PlayerState;
	if (!IsValid(PS)) return false;
	
	AGameStateBase* GS = World->GetGameState();
	if (!IsValid(GS)) return false;

	const TArray<APlayerState*>& PlayerArray = GS->PlayerArray;
	if (PlayerArray.IsEmpty()) return false;
	
	APlayerState* CurrentTargetPS = nullptr;
	AActor* CurrentViewTarget = PC->GetViewTarget();
	
	if (GetAttachParentActor())
	{
		if (APawn* ParentPawn = Cast<APawn>(GetAttachParentActor()))
		{
			CurrentTargetPS = ParentPawn->GetPlayerState();
		}
	}
	else if (APawn* TargetPawn = Cast<APawn>(CurrentViewTarget))
	{
		CurrentTargetPS = TargetPawn->GetPlayerState();
	}

	// 현재 타겟의 인덱스 찾기
	int32 CurrentIndex = -1;
	if (CurrentTargetPS)
	{
		CurrentIndex = PlayerArray.Find(CurrentTargetPS);
	}

	// 다음 인덱스부터 순회 (원형 큐처럼 순환)
	for (int32 i = 1; i <= PlayerArray.Num(); ++i)
	{
		int32 NextIndex = (CurrentIndex + i) % PlayerArray.Num();
		APlayerState* CandidatePS = PlayerArray[NextIndex];

		// 유효성 검사: 존재함 && 관전자가 아님 && 나 자신이 아님 && 폰이 있음
		if (CandidatePS && !CandidatePS->IsSpectator() && 
			CandidatePS != PC->PlayerState && CandidatePS->GetPawn())
		{
			NewViewTarget = CandidatePS->GetPawn();
			return true;
		}
	}

	return false;
}

void APTWSpectatorPawn::SetSpectatorTarget(APawn* NewViewTarget)
{
	APlayerController* PC = GetController<APlayerController>();
	if (!IsValid(PC) || !IsValid(NewViewTarget)) return;
	
	if (IsValid(CurrentViewCharacter))
	{
		CurrentViewCharacter->OnCharacterDied.RemoveDynamic(this, &ThisClass::OnTargetDeath);
		CurrentViewCharacter = nullptr;
	}
	DetachFromActor(FDetachmentTransformRules::KeepWorldTransform);
	AttachToActor(NewViewTarget, FAttachmentTransformRules::SnapToTargetNotIncludingScale);
	
	if (APTWBaseCharacter* CurrentCharacter = Cast<APTWBaseCharacter>(NewViewTarget))
	{
		CurrentViewCharacter = CurrentCharacter;
		CurrentViewCharacter->OnCharacterDied.AddUniqueDynamic(this, &ThisClass::OnTargetDeath);
	}
	
	// TODO: PTWCharacter BP의 카메라 콜리전을 IGNORE로 변경하고 삭제하면 될 것.
	if (UCapsuleComponent* TargetCapsule = NewViewTarget->FindComponentByClass<UCapsuleComponent>())
	{
		TargetCapsule->SetCollisionResponseToChannel(ECC_Camera, ECR_Ignore);
	}
	
	SetViewTarget(false);
}

void APTWSpectatorPawn::OnInputSpectateNext()
{
	if (IsLocallyControlled())
	{
		if (GetWorldTimerManager().IsTimerActive(SpectateTimer)) return;
		
		APlayerController* PC = Cast<APlayerController>(GetController());
		if (IsValid(PC) && PC->GetStateName() == NAME_Spectating)
		{
			GetWorldTimerManager().ClearTimer(SpectateTimer);
			SpectateNextPlayer();
		}
	}
}

void APTWSpectatorPawn::SwitchToFirstThirdPerson()
{
	if (GetWorldTimerManager().IsTimerActive(SpectateTimer)) return;
	
	if (IsValid(CurrentViewCharacter))
	{
		if (!bIsFirstPerson)
		{
			Current3PZoomDistance = CurrentZoomDistance;
		}
			
		bIsFirstPerson = !bIsFirstPerson;
	}
	
	SetViewTarget(true);
}

void APTWSpectatorPawn::OnTargetDeath(AActor* DeadActor, AActor* KillerActor)
{
	if (!IsLocallyControlled() && !IsValid(DeadActor)) return;
	
	GetWorldTimerManager().SetTimer(SpectateTimer, this, &APTWSpectatorPawn::SpectateNextPlayer, 2.0f, false);
}

void APTWSpectatorPawn::BlockSpectating()
{
	GetWorldTimerManager().ClearTimer(SpectateTimer);
}

void APTWSpectatorPawn::BeginPlay()
{
	if (APlayerState* PS = GetPlayerState())
	{
		UE_LOG(LogTemp, Log, TEXT("[PTWSpectatorPawn] %s: BeginPlay() called"), *PS->GetPlayerName());
	}
	else
	{
		UE_LOG(LogTemp, Log, TEXT("[PTWSpectatorPawn] BeginPlay() called"));
	}
	Super::BeginPlay();
}

void APTWSpectatorPawn::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	if (APlayerState* PS = GetPlayerState())
	{
		UE_LOG(LogTemp, Log, TEXT("[PTWSpectatorPawn] %s: EndPlay() called"), *PS->GetPlayerName());
	}
	else
	{
		UE_LOG(LogTemp, Log, TEXT("[PTWSpectatorPawn] EndPlay() called"));
	}
	
	if (IsLocallyControlled())
	{
		BlockSpectating();
		if (APTWGameState* GS = GetWorld()->GetGameState<APTWGameState>())
		{
			GS->OnMiniGameEnded.RemoveAll(this);
		}
	}
	
	Super::EndPlay(EndPlayReason);
}

void APTWSpectatorPawn::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	if (APlayerController* PC = Cast<APlayerController>(GetController()))
	{
		if (UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(PC->GetLocalPlayer()))
		{
			if (IMC_Spectator)
			{
				UE_LOG(LogTemp, Display, TEXT("IMC_Spectator"));
				Subsystem->AddMappingContext(IMC_Spectator, 0);
			}
		}
	}
	
	if (UEnhancedInputComponent* EnhancedInputComponent = Cast<UEnhancedInputComponent>(PlayerInputComponent))
	{
		if (MoveAction)
		{
			EnhancedInputComponent->BindAction(MoveAction, ETriggerEvent::Triggered, this, &ThisClass::Move);
		}
		if (LookAction)
		{
			EnhancedInputComponent->BindAction(LookAction, ETriggerEvent::Triggered, this, &ThisClass::Look);
		}
		if (ZoomAction)
		{
			EnhancedInputComponent->BindAction(ZoomAction, ETriggerEvent::Triggered, this, &ThisClass::Zoom);
		}
		if (FirstThirdPersonAction)
		{
			EnhancedInputComponent->BindAction(FirstThirdPersonAction, ETriggerEvent::Completed, this, &ThisClass::SwitchToFirstThirdPerson);
		}
		if (SpectateNextAction)
		{
			EnhancedInputComponent->BindAction(SpectateNextAction, ETriggerEvent::Completed, this, &ThisClass::OnInputSpectateNext);
		}
	}
}

void APTWSpectatorPawn::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	
	if (!FMath::IsNearlyEqual(SpringArmComponent->TargetArmLength, CurrentZoomDistance))
	{
		float NewLength = FMath::FInterpTo(
			SpringArmComponent->TargetArmLength,	// Current
			CurrentZoomDistance,					// Target
			DeltaTime,								// DeltaTime
			10.0f									// Speed (클수록 빠름)
		);

		SpringArmComponent->TargetArmLength = NewLength;
	}
}

void APTWSpectatorPawn::PossessedBy(AController* NewController)
{
	Super::PossessedBy(NewController);
	
	if (IsLocallyControlled())
	{
		BeginSpectate();
		if (APTWGameState* GS = GetWorld()->GetGameState<APTWGameState>())
		{
			GS->OnMiniGameEnded.AddUniqueDynamic(this, &ThisClass::BlockSpectating);
		}
	}
}

void APTWSpectatorPawn::OnRep_PlayerState()
{
	Super::OnRep_PlayerState();
	
	if (IsLocallyControlled())
	{
		BeginSpectate();
		if (APTWGameState* GS = GetWorld()->GetGameState<APTWGameState>())
		{
			GS->OnMiniGameEnded.AddUniqueDynamic(this, &ThisClass::BlockSpectating);
		}
	}
}
