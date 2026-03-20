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

void APTWRedLightCharacter::BeginPlay()
{
	Super::BeginPlay();
	InitialRotation = GetActorRotation();
}

APTWRedLightCharacter::APTWRedLightCharacter()
{
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
}

void APTWRedLightCharacter::SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	if (UEnhancedInputComponent* EnhancedInputComponent = Cast<UEnhancedInputComponent>(PlayerInputComponent))
	{
		if (JumpAction)
		{
			EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Started, this, &APTWRedLightCharacter::OnSpacePressed);
			EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Completed, this, &APTWRedLightCharacter::OnSpaceReleased);
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
		if (UAbilitySystemComponent* ASC = GetAbilitySystemComponent())
		{
			TArray<FGameplayAbilitySpecHandle> AbilitiesToRemove;
			for (const FGameplayAbilitySpec& Spec : ASC->GetActivatableAbilities())
			{
				AbilitiesToRemove.Add(Spec.Handle);
			}

			for (const FGameplayAbilitySpecHandle& Handle : AbilitiesToRemove)
			{
				ASC->ClearAbility(Handle);
			}

			if (AbilitiesToRemove.Num() > 0)
			{
				UE_LOG(LogTemp, Log, TEXT("[RedLight] 기존 어빌리티 %d개 초기화 완료."), AbilitiesToRemove.Num());
			}

			for (TSubclassOf<UGameplayAbility> AbilityClass : DefaultAbilities)
			{
				if (AbilityClass)
				{
					FGameplayAbilitySpec AbilitySpec(AbilityClass, 1, static_cast<int32>(INDEX_NONE), this);
					ASC->GiveAbility(AbilitySpec);
				}
			}
			UE_LOG(LogTemp, Log, TEXT("[RedLight] 술래 전용 어빌리티 %d개 부여 완료!"), DefaultAbilities.Num());
		}
		
		if (APTWPlayerState* PS = Cast<APTWPlayerState>(NewPlayerState))
		{
			if (UPTWItemSpawnManager* ItemManager = GetWorld()->GetSubsystem<UPTWItemSpawnManager>())
			{
				if (TaggerWeaponDef)
				{
					ItemManager->SpawnSingleItem(PS, TaggerWeaponDef);
					UE_LOG(LogTemp, Warning, TEXT("[RedLight] 술래에게 무기(%s) 지급 완료!"), *TaggerWeaponDef->GetName());
				}
				else
				{
					UE_LOG(LogTemp, Warning, TEXT("[RedLight] 무기 ItemDef가 선택되지 않았습니다. 캐릭터 BP를 확인해주세요."));
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
		float CalculatedDuration = FMath::Clamp(1.0f + HeldTime, 1.5f, 5.0f);

		UE_LOG(LogTemp, Log, TEXT("[술래] 입력 완료! %f초 타이머 시작"), CalculatedDuration);
		Server_StartGreenLightWithTime(CalculatedDuration);
	}
}

void APTWRedLightCharacter::Server_StartGreenLightWithTime_Implementation(float GreenLightDuration)
{
	Server_SetPhase_Implementation(ERedLightPhase::TimerPlaying);

	GetWorldTimerManager().SetTimer(
		RedLightTimerHandle,
		this,
		&APTWRedLightCharacter::TurnOnRedLight,
		GreenLightDuration,
		false
	);
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

	const float MaxChargeTime = 4.0f;

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
		if (APlayerController* PC = Cast<APlayerController>(GetController()))
		{
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
