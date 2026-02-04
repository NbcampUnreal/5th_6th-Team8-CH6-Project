// Fill out your copyright notice in the Description page of Project Settings.


#include "Gimmick/PTWLobbyPortal.h"

#include "Components/SphereComponent.h"
#include "CoreFramework/Game/GameMode/PTWGameMode.h"
#include "CoreFramework/Game/GameState/PTWGameState.h"

APTWLobbyPortal::APTWLobbyPortal()
{
	PrimaryActorTick.bCanEverTick = false;

	SphereCollision = CreateDefaultSubobject<USphereComponent>(TEXT("Sphere"));
	SphereCollision->SetupAttachment(RootComponent);
}

void APTWLobbyPortal::BeginPlay()
{
	Super::BeginPlay();

	SphereCollision->OnComponentBeginOverlap.AddDynamic(this, &APTWLobbyPortal::OnComponentBeginOverlap);
	SphereCollision->OnComponentEndOverlap.AddDynamic(this, &APTWLobbyPortal::OnComponentEndOverlap);
}

void APTWLobbyPortal::OnComponentBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
                                              UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if (!HasAuthority()) return;
	
	APTWGameState* PTWGameState = Cast<APTWGameState>(GetWorld()->GetGameState());
	if (!PTWGameState) return;

	APawn* Pawn = Cast<APawn>(OtherActor);
	if (!Pawn) return;

	APlayerState* PlayerState = Pawn->GetPlayerState();
	if (!PlayerState) return;
	
	PlayerInPortal.Add(PlayerState);
	UpdatePortalCount();
}

void APTWLobbyPortal::OnComponentEndOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
	if (!HasAuthority()) return;
	
	APTWGameState* PTWGameState = Cast<APTWGameState>(GetWorld()->GetGameState());
	if (!PTWGameState) return;

	APawn* Pawn = Cast<APawn>(OtherActor);
	if (!Pawn) return;

	APlayerState* PlayerState = Pawn->GetPlayerState();
	if (!PlayerState) return;

	PlayerInPortal.Remove(PlayerState);
	UpdatePortalCount();
}

void APTWLobbyPortal::UpdatePortalCount()
{
	if (!HasAuthority()) return;

	APTWGameState* PTWGameState = Cast<APTWGameState>(GetWorld()->GetGameState());
	if (!PTWGameState) return;

	const int32 InPortal = PlayerInPortal.Num();
	const int32 Required = PTWGameState->PlayerArray.Num() / 2 + 1;

	PTWGameState->SetPortalCount(InPortal, Required);

	if (InPortal >= Required)
	{
		APTWGameMode* GameMode = GetWorld()->GetAuthGameMode<APTWGameMode>();
		if (!GameMode) return;

		GameMode->EndTimer();
	}
}



