// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "PTWSpawnItemVolume.generated.h"

class UBoxComponent;

UCLASS()
class PTW_API APTWSpawnItemVolume : public AActor
{
	GENERATED_BODY()
	
public:
	APTWSpawnItemVolume();

protected:
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

public:
	FVector GetRandomPointInVolume() const;

protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	TObjectPtr<UBoxComponent> SpawnBox;
};
