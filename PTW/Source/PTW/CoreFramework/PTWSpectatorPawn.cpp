// Fill out your copyright notice in the Description page of Project Settings.


#include "CoreFramework/PTWSpectatorPawn.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "PTWPlayerController.h"
#include "Components/SphereComponent.h"
#include "Camera/CameraComponent.h"
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
	
	MaxZoom = 400.0f;
	MinZoom = 0.0f;
	ZoomStep = 100.0f;
	
	bIsFreeCamera = true;
	
	CurrentZoomDistance = MinZoom;
}

void APTWSpectatorPawn::SetSpectateTarget()
{
	if (bIsFirstPerson)
	{
		
	}
	else
	{
		
	}
}

void APTWSpectatorPawn::SetFirstPersonCamera()
{
	CurrentZoomDistance = 0.0f;
}

void APTWSpectatorPawn::SetThirdPersonCamera()
{
	CurrentZoomDistance = MaxZoom;
}

void APTWSpectatorPawn::Move(const FInputActionValue& Value)
{
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
	float InputValue = Value.Get<float>();
	if (FMath::IsNearlyZero(InputValue)) return;
	
	CurrentZoomDistance -= (InputValue * ZoomStep);
	CurrentZoomDistance = FMath::Clamp(CurrentZoomDistance, MinZoom, MaxZoom);
}

void APTWSpectatorPawn::SpectateNextPlayer(APawn* InOldPawn, APawn* InNewPawn)
{
	// OnPossessedPawnChanged.RemoveDynamic(this, &ThisClass::SpectateNextPlayer);

	if (IsValid(InNewPawn)) return;

	if (APawn* NewTargetView = FindNextSpectatorTarget(InNewPawn))
	{
		SetSpectatorTarget(NewTargetView);
	}
}

APawn* APTWSpectatorPawn::FindNextSpectatorTarget(APawn* InNewPawn)
{
	/*
	if (IsValid(InNewPawn)) return nullptr;
	
	APlayerController* PC = GetController<APlayerController>();
	if (!IsValid(PC)) return nullptr;
	
	APlayerState* PS = GetPlayerState();
	if (!IsValid(PS)) return nullptr;
	
	if (PS->GetPawn() || PC->GetPawn()) return nullptr;

	UWorld* World = GetWorld();
	if (!IsValid(World)) return nullptr;

	AGameStateBase* GS = World->GetGameState();
	if (!IsValid(GS)) return nullptr;

	const TArray<APlayerState*>& PlayArray = GS->PlayerArray;
	if (PlayArray.IsEmpty()) return nullptr;

	APawn* CurrentViewTarget = nullptr;
	APawn* NewViewTarget = nullptr;

	if (AActor* CurrentViewTargetActor = GetViewTarget())
	{
		CurrentViewTarget = Cast<APawn>(CurrentViewTargetActor);
		if (IsValid(CurrentViewTarget))
		{
			if (APlayerState* CurrentViewTargetPS = CurrentViewTarget->GetPlayerState())
			{
				if (!IsValid(CurrentViewTargetPS))
				{
					CurrentViewTargetPS = PS;
				}

				int32 FoundIndex = PlayArray.Find(CurrentViewTargetPS);
				if (FoundIndex != INDEX_NONE)
				{
					for (int32 i = FoundIndex + 1; i < PlayArray.Num(); i++)
					{
						if (PlayArray[i] && PlayArray[i]->GetPawn() && !PlayArray[i]->IsSpectator())
						{
							NewViewTarget = PlayArray[i]->GetPawn();
							break;
						}
					}
				}
			}
		}
	}

	if (!IsValid(NewViewTarget))
	{
		for (APlayerState* PS : PlayArray)
		{
			if (PS && (PS != PlayerState) && (!PS->IsSpectator()) && PS->GetPawn())
			{
				NewViewTarget = PS->GetPawn();
				break;
			}
		}
	}

	if (IsValid(NewViewTarget))
	{
		if (!IsValid(CurrentViewTarget) || (CurrentViewTarget != NewViewTarget))
		{
			return NewViewTarget;
		}
	}
	*/
	return nullptr;
}

void APTWSpectatorPawn::SetSpectatorTarget(APawn* NewViewTarget)
{
	TWeakObjectPtr<ThisClass> WeakThis = this;
	TWeakObjectPtr<APawn> WeakViewTarget = NewViewTarget;
	FTimerHandle NextViewTimerHandle;
	// GetWorldTimerManager().SetTimerForNextTick(NextViewTimerHandle, [WeakThis, WeakViewTarget]()
	// 	{
	// 		if (WeakThis.IsValid() && WeakViewTarget.IsValid())
	// 		{
	// 			if (APlayerController* PC = WeakThis->GetController<APlayerController>())
	// 			{
	// 				if (!IsValid(PC->GetPawn()))
	// 				{
	// 					PC->SetViewTargetWithBlend(WeakViewTarget.Get(), 0.5f, VTBlend_Cubic);
	// 				}
	// 			}
	// 		}
	// 	}, false);
}

void APTWSpectatorPawn::OnInputSpectateNext()
{
	APlayerController* PC = Cast<APlayerController>(GetController());
	if (PC->GetStateName() == NAME_Spectating)
	{
		SpectateNextPlayer(this, this);
	}
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
