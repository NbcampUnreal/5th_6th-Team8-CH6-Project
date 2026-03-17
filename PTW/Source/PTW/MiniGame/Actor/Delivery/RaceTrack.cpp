#include "RaceTrack.h"

#include "Components/SplineComponent.h"


ARaceTrack::ARaceTrack()
{
	PrimaryActorTick.bCanEverTick = false;
	
	Spline = CreateDefaultSubobject<USplineComponent>(TEXT("Spline"));
	Spline->SetupAttachment(RootComponent);
}

