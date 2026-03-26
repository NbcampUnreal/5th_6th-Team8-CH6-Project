// Fill out your copyright notice in the Description page of Project Settings.

#pragma once
#include "CoreMinimal.h"
#include "GameFramework/SpectatorPawn.h"
#include "PTWSpectatorPawn.generated.h"

class APTWBaseCharacter;
class USpringArmComponent;
class UCameraComponent;
class UInputMappingContext;
class UInputAction;
struct FInputActionValue;

UCLASS()
class PTW_API APTWSpectatorPawn : public ASpectatorPawn
{
	GENERATED_BODY()
	
public:
	APTWSpectatorPawn();
	
	void SetViewTarget(bool bIsSameViewTarget = false);
	void SetSpectatorTarget(APawn* NewViewTarget);
	bool FindNextSpectatorTarget(APawn*& NewViewTarget);
	void SpectateNextPlayer();
	
	void BeginSpectate();
	void EndSpectate();
	
	UFUNCTION()
	void StartSpectate();
	UFUNCTION()
	void OnTargetDeath(AActor* DeadActor, AActor* KillerActor);
	UFUNCTION()
	void BlockSpectating();
	
	void Move(const FInputActionValue& Value);
	void Look(const FInputActionValue& Value);
	void Zoom(const FInputActionValue& Value);
	void OnInputSpectateNext();
	void SwitchToFirstThirdPerson();
	
protected:
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
	virtual void SetupPlayerInputComponent(UInputComponent* PlayerInputComponent) override;
	virtual void Tick(float DeltaTime) override;
	virtual void PossessedBy(AController* NewController) override;
	virtual void OnRep_Controller() override;

public:
	FTimerHandle SpectateTimer;
	
protected:
	UPROPERTY(EditDefaultsOnly, Category = "Components")
	TObjectPtr<USpringArmComponent> SpringArmComponent;
	
	UPROPERTY(EditDefaultsOnly, Category = "Components")
	TObjectPtr<UCameraComponent> CameraComponent;
	
	UPROPERTY(EditDefaultsOnly, Category = "Components")
	TObjectPtr<UInputMappingContext> IMC_Spectator;
	
	UPROPERTY(EditDefaultsOnly, Category = "Input")
	TObjectPtr<UInputAction> MoveAction;
	
	UPROPERTY(EditDefaultsOnly, Category = "Input")
	TObjectPtr<UInputAction> LookAction;
	
	UPROPERTY(EditDefaultsOnly, Category = "Input")
	TObjectPtr<UInputAction> ZoomAction;
	
	UPROPERTY(EditDefaultsOnly, Category = "Input")
	TObjectPtr<UInputAction> FirstThirdPersonAction;
	
	UPROPERTY(EditDefaultsOnly, Category = "Input")
	UInputAction* SpectateNextAction;
	
	// TObjectPtr<ACharacter> CurrentViewTarget;
	
	/* 현재 줌 값 */
	UPROPERTY(VisibleAnywhere, Category = "ThirdPerson")
	float CurrentZoomDistance;
	float Current3PZoomDistance;
	float Starting3PZoomDistance;
	
	/* 최대 줌 길이 */
	UPROPERTY(EditDefaultsOnly, Category = "ThirdPerson|Zoom", meta=(ClampMin="100.0", ClampMax="1000.0"))
	float MaxZoom;
	
	/* 최소 줌 길이 (1인칭은 개별적인 '0'값을 사용) */
	UPROPERTY(EditDefaultsOnly, Category = "ThirdPerson|Zoom", meta=(ClampMin="100.0", ClampMax="1000.0"))
	float MinZoom;
	
	/* 한번에 줌이 될 길이 */
	UPROPERTY(EditDefaultsOnly, Category = "ThirdPerson|Zoom", meta=(ClampMin="100.0", ClampMax="1000.0"))
	float ZoomStep;
	
	bool bIsFreeCamera;
	bool bIsFirstPerson;
	
private:
	TObjectPtr<APTWBaseCharacter> CurrentViewCharacter;
};
