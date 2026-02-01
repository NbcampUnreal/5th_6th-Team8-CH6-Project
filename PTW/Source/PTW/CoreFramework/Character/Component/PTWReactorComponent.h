// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "GameplayTagContainer.h"
#include "PTWReactorComponent.generated.h"

class UAnimMontage;

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class PTW_API UPTWReactorComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UPTWReactorComponent();

protected:
	virtual void BeginPlay() override;

public:
	void ProcessDeath();

	UFUNCTION(NetMulticast, Unreliable)
	void Multicast_PlayHitReact(const FVector& ImpactPoint);

protected:
	UFUNCTION(NetMulticast, Reliable)
	void Multicast_Death();

	void OnTagChanged(const FGameplayTag Tag, int32 NewCount);

public:
	UPROPERTY(EditDefaultsOnly, Category = "Combat|Animation")
	TObjectPtr<UAnimMontage> HitReact_Front;
	UPROPERTY(EditDefaultsOnly, Category = "Combat|Animation")
	TObjectPtr<UAnimMontage> HitReact_Back;
	UPROPERTY(EditDefaultsOnly, Category = "Combat|Animation")
	TObjectPtr<UAnimMontage> HitReact_Left;
	UPROPERTY(EditDefaultsOnly, Category = "Combat|Animation")
	TObjectPtr<UAnimMontage> HitReact_Right;

	bool IsInputInverted() const { return bIsInputInverted; }

private:
	bool bIsInputInverted = false;
};
