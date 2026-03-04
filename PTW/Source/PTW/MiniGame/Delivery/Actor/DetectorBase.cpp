#include "DetectorBase.h"
#include "Components/BoxComponent.h"

ADetectorBase::ADetectorBase()
{
	PrimaryActorTick.bCanEverTick = false;
	
	DetectArea = CreateDefaultSubobject<UBoxComponent>(TEXT("DetectArea"));
	DetectArea->SetupAttachment(RootComponent);
	DetectArea->OnComponentBeginOverlap.AddDynamic(this, &ADetectorBase::OnDetectPlayer);
}

void ADetectorBase::OnDetectPlayer(UPrimitiveComponent* OverlappedComp,
	AActor* OtherActor, 
	UPrimitiveComponent* OtherComp, 
	int32 OtherBodyIndex, 
	bool bFromSweep, 
	const FHitResult& SweepResult)
{
	
}


