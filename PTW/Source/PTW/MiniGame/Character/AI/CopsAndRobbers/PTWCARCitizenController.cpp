#include "PTWCARCitizenController.h"
#include "PTWCARCitizen.h"
#include "NavigationSystem.h"

APTWCARCitizenController::APTWCARCitizenController()
{
	PrimaryActorTick.bCanEverTick = false;
	
	MoveRadius = 1000.0f;
}

void APTWCARCitizenController::BeginPlay()
{
	Super::BeginPlay();
	
	if (HasAuthority())
	{
		GetWorldTimerManager().SetTimer(AIMoveTimerHandle, 
			this, 
			&ThisClass::MoveToRandomLocation, 
			FMath::RandRange(3.0f, 7.0f),
			true);
	}
}

void APTWCARCitizenController::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

void APTWCARCitizenController::MoveToRandomLocation()
{
	ACharacter* TempCharacter = GetCharacter();
	if (!IsValid(TempCharacter)) return;

	APTWCARCitizen* AICharacter = Cast<APTWCARCitizen>(TempCharacter);
	if (!IsValid(AICharacter)) return;

	UNavigationSystemV1* NavigationSystem = UNavigationSystemV1::GetCurrent(GetWorld());
	if (!IsValid(NavigationSystem)) return;

	FNavLocation RandomLocation;
	bool bFoundLocation = NavigationSystem->GetRandomReachablePointInRadius(
		AICharacter->GetNavAgentLocation(), MoveRadius, RandomLocation);
	if (!bFoundLocation) return;
	
	MoveToLocation(RandomLocation.Location);
}
