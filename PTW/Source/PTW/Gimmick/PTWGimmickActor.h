// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "PTWGimmickActor.generated.h"

// 기믹 액터 플레이어와 Overlap시 GameplayEvent 발생시키는 트리거 역할.

class USphereComponent;
class UStaticMeshComponent;

UCLASS()
class PTW_API APTWGimmickActor : public AActor
{
	GENERATED_BODY()

public:
	APTWGimmickActor();

protected:
	virtual void BeginPlay() override;

	UPROPERTY(VisibleAnywhere, Category="Gimmick")
	TObjectPtr<UStaticMeshComponent> Mesh;

	UPROPERTY(VisibleAnywhere, Category="Gimmick")
	TObjectPtr<USphereComponent> Collision;

	bool bConsumed = false;

	UFUNCTION()
	void OnOverlapBegin(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
		UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);
};

