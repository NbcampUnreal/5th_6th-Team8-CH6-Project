// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "CoreFramework/PTWPlayerCharacter.h"
#include "PTWRedLightCharacter.generated.h"

class UPTWRedLightMark;
class USpotLightComponent;

UCLASS()
class PTW_API APTWRedLightCharacter : public APTWPlayerCharacter
{
	GENERATED_BODY()
	
public:
	APTWRedLightCharacter();
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "RedLight")
	TSubclassOf<UPTWRedLightMark> MarkWidgetClass;

	UPROPERTY(ReplicatedUsing = OnRep_IsRedLight, BlueprintReadOnly, Category = "RedLight")
	bool bIsRedLight = false;

	UFUNCTION(NetMulticast, Reliable)
	void Multicast_SpottedPlayer(ACharacter* CaughtPlayer);

protected:
	// 연출용 스포트라이트 (빨간 눈)
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "RedLight|Lights")
	TObjectPtr<USpotLightComponent> LeftEyeLight;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "RedLight|Lights")
	TObjectPtr<USpotLightComponent> RightEyeLight;

	UFUNCTION()
	void OnRep_IsRedLight();

	// 상태 전환 및 연출 업데이트
	void ToggleLight();
	void UpdateEyeLights();

	UFUNCTION(Server, Reliable)
	void Server_SetLightState(bool bNewState);
};
