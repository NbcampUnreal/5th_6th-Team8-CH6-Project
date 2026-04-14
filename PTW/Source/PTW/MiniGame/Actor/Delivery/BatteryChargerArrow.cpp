#include "BatteryChargerArrow.h"


ABatteryChargerArrow::ABatteryChargerArrow()
{
	PrimaryActorTick.bCanEverTick = true;
	
	SceneComponent = CreateDefaultSubobject<USceneComponent>("SceneComponent");
	SetRootComponent(SceneComponent);
	
	StaticMeshComponent = CreateDefaultSubobject<UStaticMeshComponent>("StaticMeshComponent");
	StaticMeshComponent->SetupAttachment(SceneComponent);
}

void ABatteryChargerArrow::BeginPlay()
{
	Super::BeginPlay();
	
	InitialLocation = GetActorLocation();
}

void ABatteryChargerArrow::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	
	FVector NewLocation = InitialLocation;
	
	float DeltaZ = FMath::Sin(GetWorld()->GetTimeSeconds() * Frequency) * Amplitude;
    
	NewLocation.Z += DeltaZ;
	SetActorLocation(NewLocation);
}

