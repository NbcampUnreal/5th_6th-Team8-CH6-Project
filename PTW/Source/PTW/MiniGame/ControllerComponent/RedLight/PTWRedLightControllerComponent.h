// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "MiniGame/ControllerComponent/PTWBaseControllerComponent.h"
#include "PTWRedLightControllerComponent.generated.h"

class UPTWRedLightChargeWidget;
class UUserWidget;

UCLASS(Blueprintable, ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class PTW_API UPTWRedLightControllerComponent : public UPTWBaseControllerComponent
{
	GENERATED_BODY()
	
public:
	UFUNCTION(BlueprintCallable, Category = "RedLight|UI")
	void ShowTaggerUI();
	UFUNCTION(BlueprintCallable, Category = "RedLight|UI")
	void HideTaggerUI();

protected:
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

protected:
	UPROPERTY(EditDefaultsOnly, Category = "UI")
	TSubclassOf<UPTWRedLightChargeWidget> TaggerUIClass;

	UPROPERTY()
	TObjectPtr<UPTWRedLightChargeWidget> TaggerUIInstance;
};
