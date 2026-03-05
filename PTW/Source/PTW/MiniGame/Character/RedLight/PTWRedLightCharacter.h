// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "CoreFramework/PTWBaseCharacter.h"
#include "PTWRedLightCharacter.generated.h"

class UCameraComponent;
class UPTWRedLightMark;
class USpotLightComponent;

UCLASS()
class PTW_API APTWRedLightCharacter : public APTWBaseCharacter
{
	GENERATED_BODY()
	
public:
	APTWRedLightCharacter();
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;
	
	UFUNCTION(Server, Reliable)
	void Server_SetLightState(bool bNewState);
	UFUNCTION(NetMulticast, Reliable)
	void Multicast_SpottedPlayer(ACharacter* CaughtPlayer);

protected:
	UFUNCTION()
	void OnRep_IsRedLight();

	void ToggleLight();
	void UpdateEyeLights();

public:
	UPROPERTY(ReplicatedUsing = OnRep_IsRedLight, BlueprintReadOnly, Category = "RedLight")
	bool bIsRedLight = false;

protected:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "RedLight")
	TSubclassOf<class UPTWRedLightMark> MarkWidgetClass;


	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Camera")
	UCameraComponent* FirstPersonCamera;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "RedLight|Lights")
	USpotLightComponent* LeftEyeLight;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "RedLight|Lights")
	USpotLightComponent* RightEyeLight;

};
