// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "PTWDeveloperComponent.generated.h"


UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class PTW_API UPTWDeveloperComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UPTWDeveloperComponent();

	UFUNCTION(Server, Reliable)
	void Server_ExecuteDevCommand(FName CommandName, int32 IntParam = 0, float FloatParam = 0.f, const FString& StringParam = TEXT(""));
};
