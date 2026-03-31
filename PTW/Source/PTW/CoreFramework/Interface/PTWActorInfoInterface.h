// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "PTWActorInfoInterface.generated.h"

UINTERFACE(MinimalAPI)
class UPTWActorInfoInterface : public UInterface
{
	GENERATED_BODY()
};

class PTW_API IPTWActorInfoInterface
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "ActorInfo")
	FText GetDisplayName() const;
};
