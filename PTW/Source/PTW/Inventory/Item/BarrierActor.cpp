#include "BarrierActor.h"

#include "Components/BoxComponent.h"


ABarrierActor::ABarrierActor()
{
	PrimaryActorTick.bCanEverTick = false;
	bReplicates = true;
	
	BoxComponent = CreateDefaultSubobject<UBoxComponent>(TEXT("BoxComponent"));
	SetRootComponent(BoxComponent);
	
	MeshComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("MeshComponent"));
	MeshComponent->SetupAttachment(BoxComponent);
}

// Called when the game starts or when spawned
void ABarrierActor::BeginPlay()
{
	Super::BeginPlay();
	
	FTimerDelegate TimerDelegate;
	TimerDelegate.BindLambda([this]() {this->Destroy();});
	
	GetWorldTimerManager().SetTimer(TimerHandle, TimerDelegate, 5.0f, false);
}


