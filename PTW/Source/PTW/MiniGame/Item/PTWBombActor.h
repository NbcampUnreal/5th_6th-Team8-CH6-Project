// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "AbilitySystemInterface.h"
#include "GameplayTagContainer.h"
#include "PTWBombActor.generated.h"

class UAbilitySystemComponent;
class UStaticMeshComponent;
class USphereComponent;

UCLASS()
class PTW_API APTWBombActor : public AActor, public IAbilitySystemInterface
{
	GENERATED_BODY()

public:
	APTWBombActor();
	
	virtual UAbilitySystemComponent* GetAbilitySystemComponent() const override;

protected:
	virtual void BeginPlay() override;

	/** GAS */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="GAS")
	TObjectPtr<UAbilitySystemComponent> AbilitySystemComponent;

	/** Components */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Components")
	TObjectPtr<UStaticMeshComponent> BombMesh;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Components")
	TObjectPtr<USphereComponent> Collision;

	/** 폭탄 상태 태그  */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="GAS")
	FGameplayTag BombStateTag;
};
