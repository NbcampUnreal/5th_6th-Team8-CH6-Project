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
class UCameraComponent;

UCLASS()
class PTW_API APTWRedLightCharacter : public APTWPlayerCharacter
{
	GENERATED_BODY()
	
public:
	APTWRedLightCharacter();
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;
	virtual void Tick(float DeltaTime) override;

	UFUNCTION(NetMulticast, Reliable)
	void Multicast_SpottedPlayer(ACharacter* CaughtPlayer);

	UFUNCTION(BlueprintPure, Category = "RedLight|Charge")
	float GetChargeProgress() const;

	UFUNCTION(NetMulticast, Reliable)
	void Multicast_RemoveSpottedMark(ACharacter* CaughtPlayer);

	UFUNCTION(NetMulticast, Reliable)
	void Multicast_PlayLoopSound();

	UFUNCTION(NetMulticast, Reliable)
	void Multicast_PlayEndSound(float PitchMultiplier);

	void StartZoom();
	void StopZoom();


protected:
	virtual void BeginPlay() override;
	virtual void OnPlayerStateChanged(APlayerState* NewPlayerState, APlayerState* OldPlayerState) override;
	virtual void PawnClientRestart() override;
	virtual void PossessedBy(AController* NewController) override;

	UFUNCTION(Server, Reliable)
	void Server_SetPhase(ERedLightPhase NewPhase);
	UFUNCTION(Server, Reliable)
	void Server_StartGreenLightWithTime(float HeldTime);
	UFUNCTION()
	void OnRep_CurrentPhase();

	void TurnOnRedLight();

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
	TObjectPtr<UInputAction> ChargeAction;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "RedLight|Input")
	TObjectPtr<UInputAction> ZoomAction;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "RedLight|Sound")
	TObjectPtr<USoundBase> LoopSound;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "RedLight|Sound")
	TObjectPtr<USoundBase> EndSound;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "RedLight|Battery")
	float MaxBattery = 100.0f;
	UPROPERTY(Replicated, VisibleAnywhere, BlueprintReadOnly, Category = "RedLight|Battery")
	float CurrentBattery;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "RedLight|Battery")
	float MaxBatteryCost = 30.0f;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "RedLight|Battery")
	float MaxBatteryGain = 40.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "RedLight|Zoom")
	float ZoomedFOV = 40.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "RedLight|Zoom")
	float ZoomInterpSpeed = 15.0f;

protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "RedLight|Lights")
	TObjectPtr<USpotLightComponent> LeftEyeLight;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "RedLight|Lights")
	TObjectPtr<USpotLightComponent> RightEyeLight;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "RedLight|Weapon")
	TObjectPtr<UPTWItemDefinition> TaggerWeaponDef;
	UPROPERTY(BlueprintReadOnly, Category = "RedLight|Charge")
	bool bIsCharging = false;
	UPROPERTY()
	FRotator InitialRotation;
	UPROPERTY()
	TObjectPtr<class UAudioComponent> ActiveLoopSound;

	float SpacePressedTime;
	
	FTimerHandle RedLightTimerHandle;
	FTimerHandle EndSoundTimerHandle;

	bool bIsZooming = false;
	float DefaultFOV = 90.0f;

	UPROPERTY()
	TObjectPtr<UCameraComponent> CachedCameraComp;
};
