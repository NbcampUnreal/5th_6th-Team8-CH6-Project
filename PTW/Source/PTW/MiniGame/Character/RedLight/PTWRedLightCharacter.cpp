	// Fill out your copyright notice in the Description page of Project Settings.


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

void APTWRedLightCharacter::BeginPlay()
{
	Super::BeginPlay();

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
	LeftEyeLight->SetupAttachment(GetMesh(), TEXT("head")); // 머리 소켓에 부착

	RightEyeLight = CreateDefaultSubobject<USpotLightComponent>(TEXT("RightEyeLight"));
	RightEyeLight->SetupAttachment(GetMesh(), TEXT("head"));
}

void APTWRedLightCharacter::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(APTWRedLightCharacter, bIsRedLight);
}

void APTWRedLightCharacter::SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	PlayerInputComponent->BindAction("Jump", IE_Pressed, this, &APTWRedLightCharacter::OnSpacePressed);
	PlayerInputComponent->BindAction("Jump", IE_Released, this, &APTWRedLightCharacter::OnSpaceReleased);
}

void APTWRedLightCharacter::UpdateEyeLights()
{
	if (LeftEyeLight) LeftEyeLight->SetVisibility(bIsRedLight);
	if (RightEyeLight) RightEyeLight->SetVisibility(bIsRedLight);
}

void APTWRedLightCharacter::ToggleLight()
{
	if (HasAuthority())
	{
		UE_LOG(LogTemp, Warning, TEXT("[RedLight_Server] ToggleLight 호출됨! 변경 시도..."));
		Server_SetLightState_Implementation(!bIsRedLight);
	}
}

void APTWRedLightCharacter::Server_SetLightState_Implementation(bool bNewState)
{
	bIsRedLight = bNewState;

	UE_LOG(LogTemp, Warning, TEXT("[RedLight_Server] 상태 변경 완료! 현재 상태: %s"), bIsRedLight ? TEXT("빨간불 (움직이면 죽음)") : TEXT("파란불 (안전)"));

	UpdateEyeLights();

	if (APTWRedLightGameMode* GM = Cast<APTWRedLightGameMode>(GetWorld()->GetAuthGameMode()))
	{
		GM->OnRedLightStateChanged(bIsRedLight, this);
	}
}

void APTWRedLightCharacter::OnRep_IsRedLight()
{
	UE_LOG(LogTemp, Log, TEXT("[RedLight_Client] 서버로부터 상태 동기화 수신! 현재 상태: %s"), bIsRedLight ? TEXT("빨간불") : TEXT("파란불"));
	
	UpdateEyeLights();
}

void APTWRedLightCharacter::Multicast_SpottedPlayer_Implementation(ACharacter* CaughtPlayer)
{
	if (!CaughtPlayer || !MarkWidgetClass) return;

	UWidgetComponent* MarkComp = NewObject<UWidgetComponent>(CaughtPlayer);
	MarkComp->RegisterComponent();
	MarkComp->AttachToComponent(CaughtPlayer->GetRootComponent(), FAttachmentTransformRules::KeepRelativeTransform);

	MarkComp->SetRelativeLocation(FVector(0.f, 0.f, 100.f));
	MarkComp->SetWidgetSpace(EWidgetSpace::Screen);
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
	if (IsLocallyControlled() && bIsRedLight)
	{
		SpacePressedTime = GetWorld()->GetTimeSeconds();
		UE_LOG(LogTemp, Log, TEXT("[술래] 파란불 차지 시작..."));
	}
}

void APTWRedLightCharacter::OnSpaceReleased()
{
	if (IsLocallyControlled() && bIsRedLight)
	{
		float HeldTime = GetWorld()->GetTimeSeconds() - SpacePressedTime;

		float CalculatedDuration = FMath::Clamp(1.0f + HeldTime, 1.5f, 5.0f);

		UE_LOG(LogTemp, Log, TEXT("[술래] 파란불 차지 완료! 요청 시간: %f초"), CalculatedDuration);

		Server_StartGreenLightWithTime(CalculatedDuration);
	}
}

void APTWRedLightCharacter::Server_StartGreenLightWithTime_Implementation(float GreenLightDuration)
{
	Server_SetLightState_Implementation(false);

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
		Server_SetLightState_Implementation(true);
		UE_LOG(LogTemp, Warning, TEXT("[서버] 빨간불 전환 완료!"));
	}
}
