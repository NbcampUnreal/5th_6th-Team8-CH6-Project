// Fill out your copyright notice in the Description page of Project Settings.

#pragma once
#include "CoreMinimal.h"
#include "GameFramework/SpectatorPawn.h"
#include "PTWSpectatorPawn.generated.h"

class APTWBaseCharacter;
/**
 * 
 */
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
	
	void SetSpectateTarget();
	
	void SetFirstPersonCamera();
	void SetThirdPersonCamera();
	
	void Move(const FInputActionValue& Value);
	void Look(const FInputActionValue& Value);
	
	void Zoom(const FInputActionValue& Value);
	
	void LookUp();
	void Turn();
	
	UFUNCTION()
	void SpectateNextPlayer();
	void BeginSpectate();
	bool FindNextSpectatorTarget(APawn*& NewViewTarget);
	void SetSpectatorTarget(APawn* NewViewTarget);
	UFUNCTION()
	void OnInputSpectateNext();
	UFUNCTION()
	void OnTargetDeath(AActor* DeadActor, AActor* KillerActor);
	UFUNCTION()
	void ClearAllTimer();
protected:
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
	virtual void SetupPlayerInputComponent(UInputComponent* PlayerInputComponent) override;
	virtual void Tick(float DeltaTime) override;
	virtual void PossessedBy(AController* NewController) override;
	virtual void OnRep_PlayerState() override;
	
public:
	FTimerHandle BeginSpectateTimer;
	
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
	UInputAction* SpectateNextAction;
	
	// TObjectPtr<ACharacter> CurrentViewTarget;
	
	UPROPERTY(VisibleAnywhere, Category = "Variables")
	float CurrentZoomDistance;
	
	UPROPERTY(EditDefaultsOnly, Category = "Variables")
	float MaxZoom;
	UPROPERTY(EditDefaultsOnly, Category = "Variables")
	float MinZoom;
	UPROPERTY(EditDefaultsOnly, Category = "Variables")
	float ZoomStep;
	
	bool bIsFreeCamera;
	bool bIsFirstPerson;
	
private:
	TObjectPtr<APTWBaseCharacter> CurrentViewCharacter;
};
