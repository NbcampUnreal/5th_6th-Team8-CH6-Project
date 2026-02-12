// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "PTWPickupItemBase.generated.h"

UCLASS(Abstract)
class PTW_API APTWPickupItemBase : public AActor
{
	GENERATED_BODY()
	
public:
	APTWPickupItemBase();

protected:
	virtual void BeginPlay() override;

	UFUNCTION()
	virtual void OnSphereOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

	virtual void OnPickedUp(class APTWPlayerCharacter* Player);

protected:
	UPROPERTY(EditAnywhere, Category = "Effect")
	TObjectPtr<USoundBase> PickupSound;
	UPROPERTY(EditAnywhere, Category = "Effect")
	TObjectPtr<UParticleSystem> PickupVFX;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<class USphereComponent> SphereComp;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<class UStaticMeshComponent> MeshComp;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<class URotatingMovementComponent> RotationComp;
};
