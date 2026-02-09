// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "PTWWeaponCasting.generated.h"

UCLASS()
class PTW_API APTWWeaponCasting : public AActor
{
	GENERATED_BODY()

public:
	APTWWeaponCasting();

protected:
	virtual void BeginPlay() override;
	
	UFUNCTION()
	virtual void OnHit(UPrimitiveComponent* HitComp, AActor* OtherActor, UPrimitiveComponent* OtherComp,
		FVector NormalImpulse, const FHitResult& Hit);

protected:
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Defalut|Component")
	TObjectPtr<USceneComponent> SceneComponent;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Defalut|Component")
	TObjectPtr<UStaticMeshComponent> CastingMeshComponent;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Defalut|Setting")
	float DestroyTimer;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Defalut|Setting")
	TObjectPtr<USoundBase> CastingDropSound;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Defalut|Setting")
	bool bIsSoundPlay;
};
