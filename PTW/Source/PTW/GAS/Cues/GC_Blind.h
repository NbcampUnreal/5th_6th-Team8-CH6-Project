// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameplayCueNotify_Actor.h"
#include "GC_Blind.generated.h"

class UMaterialInterface;
class UPostProcessComponent;
class UMaterialInstanceDynamic;

UCLASS()
class PTW_API AGC_Blind : public AGameplayCueNotify_Actor
{
	GENERATED_BODY()

public:
	AGC_Blind();

protected:
	virtual void BeginPlay() override;
	
	virtual bool OnActive_Implementation(AActor* MyTarget, const FGameplayCueParameters& Parameters) override;
	
	virtual bool OnRemove_Implementation(AActor* MyTarget, const FGameplayCueParameters& Parameters) override;
	
protected:
	UPROPERTY(EditDefaultsOnly, Category = "Post Process")
	TObjectPtr<UMaterialInterface> PostProcessMaterial;

	UPROPERTY()
	TObjectPtr<UPostProcessComponent> PostProcessComponent;
	
	UPROPERTY()
	TObjectPtr<UMaterialInstanceDynamic> DynamicMaterial;
	
	UPROPERTY(EditDefaultsOnly, Category = "Blind Effect")
	FName OpacityParamName = FName("BlindOpacity");
	
	UPROPERTY(EditDefaultsOnly, Category = "Blind Effect")
	float FadeSpeed = 5.0f;

	float CurrentOpacity = 0.0f;
	float TargetOpacity = 0.0f;
	
	bool bIsFadingOut = false;
	
public:
	virtual void Tick(float DeltaTime) override;
};
