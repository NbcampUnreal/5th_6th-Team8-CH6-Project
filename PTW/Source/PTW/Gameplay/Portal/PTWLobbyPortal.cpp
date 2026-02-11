// Fill out your copyright notice in the Description page of Project Settings.


#include "Gameplay/Portal/PTWLobbyPortal.h"


#include "Components/BoxComponent.h"
#include "CoreFramework/Game/GameMode/PTWGameMode.h"
#include "CoreFramework/Game/GameState/PTWGameState.h"
#include "Net/UnrealNetwork.h"

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

	if (bEnter)
	{
		PlayerInPortal.Add(PlayerState);  
	}
	else
	{
		PlayerInPortal.Remove(PlayerState);
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



