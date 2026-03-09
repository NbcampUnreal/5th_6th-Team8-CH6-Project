// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "PTWCitizenSpawner.generated.h"

class UBoxComponent;
class APTWCARCitizen;

UCLASS()
class PTW_API APTWCitizenSpawner : public AActor
{
	GENERATED_BODY()

public:
	APTWCitizenSpawner();
	void SpawningRandomLocation();
	TArray<APTWCARCitizen*>& GetCitizenInstances() { return CitizenInstances; };
	
protected:
	virtual void BeginPlay() override;
	virtual void Tick(float DeltaTime) override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
	
protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Spawning")
	UBoxComponent* SpawnVolume;
	
	UPROPERTY(EditDefaultsOnly, Category = "Spawning")
	TSubclassOf<APTWCARCitizen> CitizenClass;
	
	UPROPERTY()
	TArray<APTWCARCitizen*> CitizenInstances;
	
	UPROPERTY(EditAnywhere, Category = "Spawning")
	int32 SpawnCount;
};
