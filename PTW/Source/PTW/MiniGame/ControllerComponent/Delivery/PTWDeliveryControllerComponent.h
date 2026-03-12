// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "PTWDeliveryControllerComponent.generated.h"


UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class PTW_API UPTWDeliveryControllerComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	// Sets default values for this component's properties
	UPTWDeliveryControllerComponent();
	void AddBatteryUI();

protected:
	virtual void BeginPlay() override;
	
	UFUNCTION(Client, Reliable)
	void ClientRPC_AddBatteryUI();

protected:
	UPROPERTY(EditAnywhere)
	TSubclassOf<UUserWidget> BatteryWidgetClass;
};
