// Fill out your copyright notice in the Description page of Project Settings.


#include "Gameplay/Portal/PTWLobbyPortal.h"


#include "Components/BoxComponent.h"
#include "CoreFramework/Game/GameMode/PTWGameMode.h"
#include "CoreFramework/Game/GameState/PTWGameState.h"
#include "Net/UnrealNetwork.h"

#include "PTWGameplayTag/GameplayTags.h"
#include "AbilitySystemComponent.h"
#include "AbilitySystemInterface.h"
#include "GameFramework/PlayerState.h"
#include "GameplayEffect.h"
#include "CoreFramework/PTWPlayerState.h"
#include "GameplayEffectTypes.h"

APTWLobbyPortal::APTWLobbyPortal()
{
	bReplicates = true;
	PrimaryActorTick.bCanEverTick = false;
	
	BoxComponent = CreateDefaultSubobject<UBoxComponent>(TEXT("Sphere"));
	RootComponent = BoxComponent;

	SetHidden(true);
	SetActorEnableCollision(false);
}

void APTWLobbyPortal::BeginPlay()
{
	Super::BeginPlay();

	BoxComponent->OnComponentBeginOverlap.AddDynamic(this, &APTWLobbyPortal::OnComponentBeginOverlap);
	BoxComponent->OnComponentEndOverlap.AddDynamic(this, &APTWLobbyPortal::OnComponentEndOverlap);
	
	APTWGameState* PTWGameState = Cast<APTWGameState>(GetWorld()->GetGameState());
	if (!PTWGameState) return;

	PTWGameState->OnGamePhaseChanged.AddDynamic(this, &APTWLobbyPortal::PortalEnable);

	
	if (PTWGameState->GetCurrentGamePhase() == EPTWGamePhase::PostGameLobby)
	{
		if (!HasAuthority()) return;
	
		SetPortalEnabled(true);
	}
	
	// 임시로 처음 로비에도 포탈 생성
	// if (!HasAuthority()) return;
	//
	// SetPortalEnabled(true);
	
}

void APTWLobbyPortal::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ThisClass, bPortalEnabled);
}

void APTWLobbyPortal::OnComponentBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
                                              UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	PortalOverlap(OtherActor, true);
}

void APTWLobbyPortal::OnComponentEndOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
	PortalOverlap(OtherActor, false);
}

void APTWLobbyPortal::UpdatePortalCount()
{
	if (!HasAuthority()) return;

	APTWGameState* PTWGameState = Cast<APTWGameState>(GetWorld()->GetGameState());
	if (!PTWGameState) return;

	const int32 InPortal = PlayerInPortal.Num();
	const int32 Required = PTWGameState->PlayerArray.Num();
	
	PTWGameState->SetPortalCount(InPortal, Required);

	if (InPortal >= Required)
	{
		APTWGameMode* GameMode = GetWorld()->GetAuthGameMode<APTWGameMode>();
		if (!GameMode) return;

		GameMode->EndTimer();
	}
}

bool APTWLobbyPortal::GetOverlappingPlayerState(AActor* OtherActor, APlayerState*& OutPlayerState) const
{
	OutPlayerState = nullptr;

	if (!HasAuthority() || !OtherActor) return false;
	
	const APawn* Pawn = Cast<APawn>(OtherActor);
	if (!Pawn) return false;
	
	APlayerState* PS = Pawn->GetPlayerState();
	if (!PS) return false;

	OutPlayerState = PS;
	return true;
}

void APTWLobbyPortal::PortalOverlap(AActor* OtherActor, bool bEnter)
{
	APlayerState* PlayerState = nullptr;
	if (!GetOverlappingPlayerState(OtherActor, PlayerState)) return;

	UAbilitySystemComponent* ASC = nullptr;
	if (APTWPlayerState* PS = Cast<APTWPlayerState>(PlayerState))
	{
		ASC = PS->GetAbilitySystemComponent();
	}

	if (bEnter)
	{
		PlayerInPortal.Add(PlayerState);  

		if (ASC)
		{
			if (InPortalEffectClass)
			{
				FGameplayEffectContextHandle Context = ASC->MakeEffectContext();

				FGameplayEffectSpecHandle Spec =
					ASC->MakeOutgoingSpec(InPortalEffectClass, 1.f, Context);

				if (Spec.IsValid())
				{
					FActiveGameplayEffectHandle Handle =
						ASC->ApplyGameplayEffectSpecToSelf(*Spec.Data.Get());

					PortalEffectHandles.Add(PlayerState, Handle);
				}
			}

			UE_LOG(LogTemp, Log, TEXT("PTWLobbyPortal: Applied Tag to %s"), *PlayerState->GetPlayerName());
		}
	}
	else
	{
		PlayerInPortal.Remove(PlayerState);

		if (ASC)
		{
			if (FActiveGameplayEffectHandle* Handle = PortalEffectHandles.Find(PlayerState))
			{
				if (Handle->IsValid())
				{
					ASC->RemoveActiveGameplayEffect(*Handle);
				}

				PortalEffectHandles.Remove(PlayerState);
			}

			UE_LOG(LogTemp, Log, TEXT("PTWLobbyPortal: Removed Tag from %s"), *PlayerState->GetPlayerName());
		}
	}

	UpdatePortalCount();
}

void APTWLobbyPortal::PortalEnable(EPTWGamePhase GamePhase)
{
	if (!HasAuthority()) return;
	
	if (GamePhase == EPTWGamePhase::PostGameLobby)
	{
		SetPortalEnabled(true);
	}
}

void APTWLobbyPortal::ApplyPortalEnabled(bool bEnable)
{
	SetActorHiddenInGame(!bEnable);
	SetActorEnableCollision(bEnable);
}

void APTWLobbyPortal::SetPortalEnabled(bool bEnable)
{
	if (!HasAuthority()) return;
	
	bPortalEnabled = bEnable;

	OnRep_PortalEnabled();

	//ForceNetUpdate();
}

void APTWLobbyPortal::OnRep_PortalEnabled()
{
	ApplyPortalEnabled(bPortalEnabled);
}



