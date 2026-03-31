// Fill out your copyright notice in the Description page of Project Settings.


#include "MiniGame/Character/RedLight/PTWRedLightCharacter.h"
#include "MiniGame/GameMode/PTWRedLightGameMode.h"
#include "MiniGame/Widget/RedLight/PTWRedLightMark.h"
#include "Camera/CameraComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Components/WidgetComponent.h"
#include "Components/SpotLightComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Net/UnrealNetwork.h"
#include "DrawDebugHelpers.h"
#include "TimerManager.h"
#include "System/PTWItemSpawnManager.h"
#include "MiniGame/ControllerComponent/RedLight/PTWRedLightControllerComponent.h"
#include "AbilitySystemComponent.h"
#include "GameplayAbilitySpec.h"
#include "EnhancedInputComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Components/AudioComponent.h"
#include "PTWGamePlayTag/GameplayTags.h" 
#include "Inventory/PTWInventoryComponent.h"
#include "CoreFramework/PTWPlayerController.h"

#define LOCTEXT_NAMESPACE "RedLightCharacter"

void APTWRedLightCharacter::BeginPlay()
{
	Super::BeginPlay();
	InitialRotation = FRotator(0.0f, 90.f, 0.f);

	CachedCameraComp = FindComponentByClass<UCameraComponent>();
	if (CachedCameraComp)
	{
		DefaultFOV = CachedCameraComp->FieldOfView;
	}

	if (HasAuthority())
	{
		CurrentBattery = MaxBattery;
	}
}

void APTWRedLightCharacter::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	Super::EndPlay(EndPlayReason);

	UpdateEyeLights();

	bUseControllerRotationYaw = true;

	if (APlayerController* PC = Cast<APlayerController>(GetController()))
	{
		PC->ResetIgnoreLookInput();
	}
}

APTWRedLightCharacter::APTWRedLightCharacter()
{
	if (GetMesh())
	{
		GetMesh()->bOwnerNoSee = true;
	}

	if (GetCharacterMovement())
	{
		GetCharacterMovement()->DisableMovement();
	}

	LeftEyeLight = CreateDefaultSubobject<USpotLightComponent>(TEXT("LeftEyeLight"));
	LeftEyeLight->SetupAttachment(GetMesh(), TEXT("head"));

	RightEyeLight = CreateDefaultSubobject<USpotLightComponent>(TEXT("RightEyeLight"));
	RightEyeLight->SetupAttachment(GetMesh(), TEXT("head"));
}

void APTWRedLightCharacter::PawnClientRestart()
{
	Super::PawnClientRestart();

	UpdateTaggerState();
}

void APTWRedLightCharacter::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(APTWRedLightCharacter, bIsRedLight);
	DOREPLIFETIME(APTWRedLightCharacter, CurrentPhase);
	DOREPLIFETIME(APTWRedLightCharacter, CurrentBattery);
}

void APTWRedLightCharacter::SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	if (UEnhancedInputComponent* EnhancedInputComponent = Cast<UEnhancedInputComponent>(PlayerInputComponent))
	{
		if (ChargeAction)
		{
			EnhancedInputComponent->BindAction(ChargeAction, ETriggerEvent::Started, this, &APTWRedLightCharacter::OnSpacePressed);
			EnhancedInputComponent->BindAction(ChargeAction, ETriggerEvent::Completed, this, &APTWRedLightCharacter::OnSpaceReleased);
		}
		if (ZoomAction)
		{
			EnhancedInputComponent->BindAction(ZoomAction, ETriggerEvent::Started, this, &APTWRedLightCharacter::StartZoom);
			EnhancedInputComponent->BindAction(ZoomAction, ETriggerEvent::Completed, this, &APTWRedLightCharacter::StopZoom);
		}
	}
}

void APTWRedLightCharacter::UpdateEyeLights()
{
	if (LeftEyeLight) LeftEyeLight->SetVisibility(bIsRedLight);
	if (RightEyeLight) RightEyeLight->SetVisibility(bIsRedLight);
}


void APTWRedLightCharacter::Server_SetPhase_Implementation(ERedLightPhase NewPhase)
{
	FString PhaseString = UEnum::GetValueAsString(NewPhase);
	UE_LOG(LogTemp, Warning, TEXT("[RedLight_Server] 상태 변경됨! NewPhase: %s"), *PhaseString);
	
	CurrentPhase = NewPhase;
	bIsRedLight = (CurrentPhase == ERedLightPhase::RedLight);

	UpdateTaggerState();

	if (CurrentPhase == ERedLightPhase::WaitInput)
	{
		Multicast_PlayLoopSound();
	}

	if (APTWRedLightGameMode* GM = Cast<APTWRedLightGameMode>(GetWorld()->GetAuthGameMode()))
	{
		GM->OnRedLightStateChanged(bIsRedLight, this);
	}

	if (CurrentPhase == ERedLightPhase::RedLight)
	{
		GetWorldTimerManager().SetTimer(
			RedLightTimerHandle,
			this,
			&APTWRedLightCharacter::OnRedLightTimerEnded,
			4.0f,
			false
		);
	}
}

void APTWRedLightCharacter::Multicast_SpottedPlayer_Implementation(ACharacter* CaughtPlayer)
{
	if (!CaughtPlayer || !MarkWidgetClass) return;

	UWidgetComponent* MarkComp = NewObject<UWidgetComponent>(CaughtPlayer);
	MarkComp->RegisterComponent();
	MarkComp->AttachToComponent(CaughtPlayer->GetRootComponent(), FAttachmentTransformRules::KeepRelativeTransform);
	MarkComp->ComponentTags.Add(FName("SpottedMark"));
	MarkComp->SetRelativeLocation(FVector(0.f, 0.f, 100.f));
	MarkComp->SetWidgetSpace(EWidgetSpace::World);
	MarkComp->SetWidgetClass(MarkWidgetClass);
	MarkComp->SetDrawSize(FVector2D(150.f, 150.f));
	MarkComp->InitWidget();

	if (UPTWRedLightMark* MarkWidget = Cast<UPTWRedLightMark>(MarkComp->GetWidget()))
	{
		MarkWidget->PlaySpottedAnimation();
	}
}

void APTWRedLightCharacter::OnPlayerStateChanged(APlayerState* NewPlayerState, APlayerState* OldPlayerState)
{
	Super::OnPlayerStateChanged(NewPlayerState, OldPlayerState);

	if (HasAuthority() && NewPlayerState)
	{
		GetWorldTimerManager().SetTimer(
			WaitInitTimerHandle,
			FTimerDelegate::CreateUObject(this, &APTWRedLightCharacter::CheckAndGiveWeapon, NewPlayerState),
			0.1f,
			true
		);
	}
}

void APTWRedLightCharacter::CheckAndGiveWeapon(APlayerState* NewPlayerState)
{
	if (bIsAbilitiesInitialized)
	{
		GetWorldTimerManager().ClearTimer(WaitInitTimerHandle);

		if (APTWPlayerState* PS = Cast<APTWPlayerState>(NewPlayerState))
		{
			if (UPTWItemSpawnManager* ItemManager = GetWorld()->GetSubsystem<UPTWItemSpawnManager>())
			{
				if (TaggerWeaponDef)
				{
					ItemManager->SpawnSingleItem(PS, TaggerWeaponDef);
					UE_LOG(LogTemp, Warning, TEXT("[RedLight] 초기화 완료! 술래에게 무기(%s) 지급 성공!"), *TaggerWeaponDef->GetName());

					if (UPTWInventoryComponent* InvComp = PS->GetInventoryComponent())
					{
						InvComp->EquipWeapon(0);
					}
				}
			}
		}
	}
}

void APTWRedLightCharacter::OnSpacePressed()
{
	if (IsLocallyControlled() && CurrentPhase == ERedLightPhase::WaitInput)
	{
		SpacePressedTime = GetWorld()->GetTimeSeconds();
		bIsCharging = true;
		UE_LOG(LogTemp, Log, TEXT("[술래] 파란불 차지 시작..."));
	}
}

void APTWRedLightCharacter::OnSpaceReleased()
{
	if (IsLocallyControlled() && bIsCharging)
	{
		bIsCharging = false;
		float HeldTime = GetWorld()->GetTimeSeconds() - SpacePressedTime;

		Server_StartGreenLightWithTime(HeldTime);
	}
}

void APTWRedLightCharacter::Server_StartGreenLightWithTime_Implementation(float HeldTime)
{
	float TargetTime = FMath::Clamp(HeldTime, 0.75f, 3.0f);
	float BaseTime = 1.5f;
	float ActualTime = TargetTime;

	if (TargetTime < BaseTime)
	{
		float TimeDiff = BaseTime - TargetTime;
		float Cost = (TimeDiff / 0.75f) * MaxBatteryCost;

		if (CurrentBattery < Cost)
		{
			float AffordableDiff = (CurrentBattery / MaxBatteryCost) * 0.75f;
			ActualTime = BaseTime - AffordableDiff;
			CurrentBattery = 0.0f;
		}
		else
		{
			CurrentBattery -= Cost;
		}
	}
	else if (TargetTime > BaseTime)
	{
		float TimeDiff = TargetTime - BaseTime;
		float Gain = (TimeDiff / 1.5f) * MaxBatteryGain;
		CurrentBattery = FMath::Clamp(CurrentBattery + Gain, 0.0f, MaxBattery);
	}

	float TotalDuration = 1.0f + ActualTime;
	float PitchMultiplier = 1.5f / ActualTime;

	UE_LOG(LogTemp, Warning, TEXT("[배터리] 남은량: %f | 클라이언트 요구: %f초 -> 최종 보정: %f초 (배속: %f)"),
		CurrentBattery, TargetTime, ActualTime, PitchMultiplier);

	Server_SetPhase_Implementation(ERedLightPhase::TimerPlaying);

	FTimerDelegate SoundDel = FTimerDelegate::CreateUObject(this, &APTWRedLightCharacter::Multicast_PlayEndSound, PitchMultiplier);
	GetWorldTimerManager().SetTimer(EndSoundTimerHandle, SoundDel, 1.0f, false);

	GetWorldTimerManager().SetTimer(RedLightTimerHandle, this, &APTWRedLightCharacter::TurnOnRedLight, TotalDuration, false);
}

void APTWRedLightCharacter::TurnOnRedLight()
{
	if (HasAuthority())
	{
		UE_LOG(LogTemp, Warning, TEXT("[서버] 타이머 종료! 빨간불 전환!"));
		Server_SetPhase_Implementation(ERedLightPhase::RedLight);
	}
}

float APTWRedLightCharacter::GetChargeProgress() const
{
	if (!bIsCharging) return 0.0f;

	float HeldTime = GetWorld()->GetTimeSeconds() - SpacePressedTime;
	const float MaxChargeTime = 3.0f;

	return FMath::Clamp(HeldTime / MaxChargeTime, 0.0f, 1.0f);
}

void APTWRedLightCharacter::UpdateTaggerState()
{
	UpdateEyeLights();

	if (bIsRedLight)
	{
		bUseControllerRotationYaw = true;
		if (GetCharacterMovement())
		{
			GetCharacterMovement()->bUseControllerDesiredRotation = true;
		}
		SetActorRotation(InitialRotation);
	}
	else
	{
		bUseControllerRotationYaw = false;
		if (GetCharacterMovement())
		{
			GetCharacterMovement()->bUseControllerDesiredRotation = false;
		}
		SetActorRotation(InitialRotation + FRotator(0.f, 180.f, 0.f));
	}

	if (IsLocallyControlled())
	{
		if (APTWPlayerController* PC = Cast<APTWPlayerController>(GetController()))
		{
			PC->ResetIgnoreLookInput();

			if (bIsRedLight)
			{
				PC->SetControlRotation(InitialRotation);
				FText TaggerMsg = LOCTEXT("RedLightTaggerMsg", "우클릭으로 확대하여 움직인 플레이어를 처치하세요!");
				PC->SendMessage(TaggerMsg, ENotificationPriority::Normal, 4);
			}
			else
			{
				FRotator WallFacingRotation = InitialRotation;
				WallFacingRotation.Yaw += 180.f;
				PC->SetControlRotation(WallFacingRotation);
				PC->SetIgnoreLookInput(true);
			}

			if (UPTWRedLightControllerComponent* UIComp = PC->FindComponentByClass<UPTWRedLightControllerComponent>())
			{
				if (bIsRedLight)
				{
					UIComp->HideTaggerUI();
				}
				else
				{
					UIComp->ShowTaggerUI();
				}
			}
		}
	}
}

void APTWRedLightCharacter::OnRep_CurrentPhase()
{
	bIsRedLight = (CurrentPhase == ERedLightPhase::RedLight);
	UpdateTaggerState();
}

void APTWRedLightCharacter::OnRedLightTimerEnded()
{
	if (HasAuthority())
	{
		UE_LOG(LogTemp, Warning, TEXT("[순환] 빨간불 종료! 다시 '입력 대기'로 돌아갑니다."));
		Server_SetPhase_Implementation(ERedLightPhase::WaitInput);
	}
}

void APTWRedLightCharacter::PossessedBy(AController* NewController)
{
	Super::PossessedBy(NewController);

	if (HasAuthority())
	{
		Server_SetPhase(ERedLightPhase::WaitInput);
	}
}

void APTWRedLightCharacter::Multicast_RemoveSpottedMark_Implementation(ACharacter* CaughtPlayer)
{
	if (!IsValid(CaughtPlayer)) return;

	TArray<UWidgetComponent*> WidgetComps;
	CaughtPlayer->GetComponents<UWidgetComponent>(WidgetComps);

	for (UWidgetComponent* Comp : WidgetComps)
	{
		if (Comp->ComponentTags.Contains(FName("SpottedMark")))
		{
			Comp->DestroyComponent();
		}
	}
}

void APTWRedLightCharacter::Multicast_PlayLoopSound_Implementation()
{
	if (LoopSound)
	{
		if (!ActiveLoopSound || !ActiveLoopSound->IsPlaying())
		{
			ActiveLoopSound = UGameplayStatics::SpawnSound2D(GetWorld(), LoopSound);
		}
	}
}

void APTWRedLightCharacter::Multicast_PlayEndSound_Implementation(float PitchMultiplier)
{
	if (ActiveLoopSound && ActiveLoopSound->IsPlaying())
	{
		ActiveLoopSound->Stop();
		ActiveLoopSound = nullptr;
	}
	
	if (EndSound)
	{
		UGameplayStatics::PlaySound2D(GetWorld(), EndSound, 1.0f, PitchMultiplier);
	}
}

void APTWRedLightCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (IsLocallyControlled() && CachedCameraComp)
	{
		float TargetFOV = bIsZooming ? ZoomedFOV : DefaultFOV;
		float CurrentFOV = CachedCameraComp->FieldOfView;

		if (!FMath::IsNearlyEqual(CurrentFOV, TargetFOV))
		{
			float NewFOV = FMath::FInterpTo(CurrentFOV, TargetFOV, DeltaTime, ZoomInterpSpeed);
			CachedCameraComp->SetFieldOfView(NewFOV);
		}
	}
}

void APTWRedLightCharacter::StartZoom()
{
	if (IsLocallyControlled())
	{
		bIsZooming = true;
	}
}

void APTWRedLightCharacter::StopZoom()
{
	if (IsLocallyControlled())
	{
		bIsZooming = false;
	}
}

void APTWRedLightCharacter::Server_ForceStopMechanics_Implementation()
{
	GetWorldTimerManager().ClearTimer(RedLightTimerHandle);
	GetWorldTimerManager().ClearTimer(EndSoundTimerHandle);

	bIsCharging = false;

	Multicast_ForceStopMechanics();
}

void APTWRedLightCharacter::Multicast_ForceStopMechanics_Implementation()
{
	if (ActiveLoopSound && ActiveLoopSound->IsPlaying())
	{
		ActiveLoopSound->Stop();
		ActiveLoopSound = nullptr;
	}

	if (IsLocallyControlled())
	{
		if (APlayerController* PC = Cast<APlayerController>(GetController()))
		{
			if (UPTWRedLightControllerComponent* UIComp = PC->FindComponentByClass<UPTWRedLightControllerComponent>())
			{
				UIComp->HideTaggerUI();
			}
		}
	}
}

#undef LOCTEXT_NAMESPACE
