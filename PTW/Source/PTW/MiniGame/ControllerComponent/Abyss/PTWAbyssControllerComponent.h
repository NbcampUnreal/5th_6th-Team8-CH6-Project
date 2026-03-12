// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "PTWAbyssControllerComponent.generated.h"

class APostProcessVolume;

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class PTW_API UPTWAbyssControllerComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UPTWAbyssControllerComponent();

	void SetAbyssDark(bool bEnable);

protected:
	virtual void BeginPlay() override;

private:
	void CacheAbyssPP();

	UPROPERTY()
	TObjectPtr<APostProcessVolume> CachedAbyssPP = nullptr;
};
