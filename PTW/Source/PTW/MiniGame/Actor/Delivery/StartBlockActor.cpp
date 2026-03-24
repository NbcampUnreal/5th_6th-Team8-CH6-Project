#include "StartBlockActor.h"

#include "Net/UnrealNetwork.h"

AStartBlockActor::AStartBlockActor()
{
	PrimaryActorTick.bCanEverTick = false;
	bReplicates = true;
	
	SceneComponent = CreateDefaultSubobject<USceneComponent>("SceneComponent");
	SetRootComponent(RootComponent);
	
	StaticMeshComponent = CreateDefaultSubobject<UStaticMeshComponent>("StaticMeshComponent");
	StaticMeshComponent->SetupAttachment(SceneComponent);
}

void AStartBlockActor::HideActor()
{
	if (HasAuthority())
	{
		bIsHidden = true;
		OnRep_IsHidden(); 
	}
}

void AStartBlockActor::GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(ThisClass, bIsHidden);
}

void AStartBlockActor::BeginPlay()
{
	Super::BeginPlay();
}

void AStartBlockActor::OnRep_IsHidden()
{
	StaticMeshComponent->SetHiddenInGame(bIsHidden);
	StaticMeshComponent->SetCollisionProfileName(bIsHidden ? TEXT("NoCollision") : TEXT("BlockAll"));
}




