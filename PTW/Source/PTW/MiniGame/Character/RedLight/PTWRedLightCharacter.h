// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "CoreFramework/PTWPlayerCharacter.h"
#include "PTWRedLightCharacter.generated.h"

UENUM(BlueprintType)
enum class ERedLightPhase : uint8
{
	WaitInput       UMETA(DisplayName = "입력대기(파란불)"),
	InputComplete   UMETA(DisplayName = "입력완료(파란불)"),
	TimerPlaying    UMETA(DisplayName = "타이머재생(파란불)"),
	RedLight        UMETA(DisplayName = "빨간불")
};

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

	UFUNCTION(BlueprintPure, Category = "RedLight|Charge")
	float GetChargeProgress() const;

	UFUNCTION(NetMulticast, Reliable)
	void Multicast_RemoveSpottedMark(ACharacter* CaughtPlayer);


protected:
	virtual void BeginPlay() override;
	virtual void OnPlayerStateChanged(APlayerState* NewPlayerState, APlayerState* OldPlayerState) override;
	virtual void PawnClientRestart() override;
	virtual void PossessedBy(AController* NewController) override;

	UFUNCTION(Server, Reliable)
	void Server_SetPhase(ERedLightPhase NewPhase);
	UFUNCTION(Server, Reliable)
	void Server_StartGreenLightWithTime(float GreenLightDuration);

	UFUNCTION()
	void OnRep_CurrentPhase();

	void TurnOnRedLight();

	void ToggleLight();
	void UpdateEyeLights();

	void OnSpacePressed();
	void OnSpaceReleased();

	void UpdateTaggerState();

	void OnRedLightTimerEnded();

public:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "RedLight")
	TSubclassOf<UPTWRedLightMark> MarkWidgetClass;

	UPROPERTY(Replicated, BlueprintReadOnly, Category = "RedLight")
	bool bIsRedLight = false;

	UPROPERTY(ReplicatedUsing = OnRep_CurrentPhase, BlueprintReadOnly, Category = "RedLight")
	ERedLightPhase CurrentPhase = ERedLightPhase::WaitInput;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "RedLight|Input")
	class UInputAction* JumpAction;
protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "RedLight|Lights")
	TObjectPtr<USpotLightComponent> LeftEyeLight;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "RedLight|Lights")
	TObjectPtr<USpotLightComponent> RightEyeLight;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "RedLight|Weapon")
	TObjectPtr<class UPTWItemDefinition> TaggerWeaponDef;
	UPROPERTY(BlueprintReadOnly, Category = "RedLight|Charge")
	bool bIsCharging = false;
	UPROPERTY()
	FRotator InitialRotation;

	float SpacePressedTime;
	
	FTimerHandle RedLightTimerHandle;


};
