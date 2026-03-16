// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "MiniGame/ControllerComponent/PTWBaseControllerComponent.h"
#include "PTWDeliveryControllerComponent.generated.h"

class UPTWDeliveryHUD;
class UPTWBatterLevelWidget;

UCLASS(Blueprintable, ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class PTW_API UPTWDeliveryControllerComponent : public UPTWBaseControllerComponent
{
	GENERATED_BODY()

public:
	// Sets default values for this component's properties
	UPTWDeliveryControllerComponent();
	void AddBatteryUI();
	void ShowCountDownWidget();
	void SetCountDownText(int32 Count);

protected:
	virtual void BeginPlay() override;
	
	UFUNCTION(Client, Reliable)
	void ClientRPC_AddBatteryUI();
	
	UFUNCTION(Client, Reliable)
	void ClientRPC_ShowCountDownWidget();
	
	UFUNCTION(Client, Reliable)
	void ClientRPC_SetCountDownText(int32 Count);

protected:
	UPROPERTY(EditAnywhere)
	TSubclassOf<UUserWidget> DeliveryHUDClass;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	TObjectPtr<UPTWDeliveryHUD> DeliveryHUDWidgetInstance;
};
