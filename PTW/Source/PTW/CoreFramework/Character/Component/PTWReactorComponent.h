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

	/* 플레이어 피격효과 방향에 따라 재생 */
	void HitReact(const FVector& ImpactPoint);

	/* 플레이어 사망 처리 */
	void ProcessDeath();

protected:
	virtual void BeginPlay() override;

	/* 플레이어 사망 처리 Multicast */
	UFUNCTION(NetMulticast, Reliable)
	void Multicast_Death();

	/* 플레이어 피격효과 방향에 따라 재생 Multicast */
	UFUNCTION(NetMulticast, Unreliable)
	void Multicast_PlayHitReact(const FVector& ImpactPoint);

public:
	UPROPERTY(EditDefaultsOnly, Category = "Combat|Animation")
	TObjectPtr<UAnimMontage> HitReact_Front;
	UPROPERTY(EditDefaultsOnly, Category = "Combat|Animation")
	TObjectPtr<UAnimMontage> HitReact_Back;
	UPROPERTY(EditDefaultsOnly, Category = "Combat|Animation")
	TObjectPtr<UAnimMontage> HitReact_Left;
	UPROPERTY(EditDefaultsOnly, Category = "Combat|Animation")
	TObjectPtr<UAnimMontage> HitReact_Right;

};
