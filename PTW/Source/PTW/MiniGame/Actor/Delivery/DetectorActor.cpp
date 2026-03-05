#include "DetectorActor.h"

#include "Components/BoxComponent.h"

ADetectorActor::ADetectorActor()
{
	PrimaryActorTick.bCanEverTick = false;
	bReplicates = true;
	
	SceneComponent = CreateDefaultSubobject<USceneComponent>("SceneComponent");
	SetRootComponent(SceneComponent);
	
	BoxComponent = CreateDefaultSubobject<UBoxComponent>("BoxComponent");
	BoxComponent->SetupAttachment(SceneComponent);
	
	BoxComponent->OnComponentBeginOverlap.AddDynamic(this, &ADetectorActor::OnDetectOverlap);
}

void ADetectorActor::OnDetectOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	
}

