// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "CoreFramework/PTWPlayerCharacter.h"
#include "PTWRedLightCharacter.generated.h"

class UPTWRedLightMark;
class USpotLightComponent;
class UPTWItemDefinition;

UCLASS()
class PTW_API APTWRedLightCharacter : public APTWPlayerCharacter
{
	GENERATED_BODY()
	
public:
	APTWRedLightCharacter();
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

	UFUNCTION(NetMulticast, Reliable)
	void Multicast_SpottedPlayer(ACharacter* CaughtPlayer);


protected:
	virtual void BeginPlay() override;
	virtual void OnPlayerStateChanged(APlayerState* NewPlayerState, APlayerState* OldPlayerState) override;

	UFUNCTION(Server, Reliable)
	void Server_SetLightState(bool bNewState);
	UFUNCTION(Server, Reliable)
	void Server_StartGreenLightWithTime(float GreenLightDuration);
	UFUNCTION()
	void OnRep_IsRedLight();

	void TurnOnRedLight();

	void ToggleLight();
	void UpdateEyeLights();

	void OnSpacePressed();
	void OnSpaceReleased();

public:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "RedLight")
	TSubclassOf<UPTWRedLightMark> MarkWidgetClass;

	UPROPERTY(ReplicatedUsing = OnRep_IsRedLight, BlueprintReadOnly, Category = "RedLight")
	bool bIsRedLight = false;

protected:
	// 연출용 스포트라이트 (빨간 눈)
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "RedLight|Lights")
	TObjectPtr<USpotLightComponent> LeftEyeLight;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "RedLight|Lights")
	TObjectPtr<USpotLightComponent> RightEyeLight;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "RedLight|Weapon")
	TObjectPtr<class UPTWItemDefinition> TaggerWeaponDef;

	float SpacePressedTime;
	
	FTimerHandle RedLightTimerHandle;


};
