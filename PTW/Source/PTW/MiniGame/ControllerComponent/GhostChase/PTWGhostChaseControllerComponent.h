// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "PTWGhostChaseControllerComponent.generated.h"

class UWidgetComponent;
class UUserWidget;
class UTextureRenderTarget2D;
class USceneCaptureComponent2D;

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class PTW_API UPTWGhostChaseControllerComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	UPTWGhostChaseControllerComponent();

	void SetTarget(APawn* NewTarget);
	APawn* GetTarget() const { return CurrentTargetPawn; }

	bool IsTarget(APawn* Pawn) const;

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	UFUNCTION()
	void OnRep_CurrentTargetPawn();

	/* POV 관련 함수 */
	void UpdateTargetPOV(APawn* NewTarget);
	void RefreshTargetViewHiddenActors();
	void CaptureTargetPOV();
	UFUNCTION(Client, Reliable)
	void Client_UpdateTargetPOV(APawn* NewTarget);

	/* 이름 강조 */
	void ApplyNameTagHighlight(APawn* TargetPawn, UWidgetComponent* WidgetComp);

protected:
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

	UPROPERTY(Replicated)
	TObjectPtr<APawn> CurrentTargetPawn;
	
	UPROPERTY()
	TObjectPtr<UTextureRenderTarget2D> TargetPOVRT;

	UPROPERTY()
	TObjectPtr<USceneCaptureComponent2D> CurrentActiveCapture;

	FTimerHandle POVCaptureTimerHandle;

	UPROPERTY(EditAnywhere, Category = "GhostChase")
	TSubclassOf<UUserWidget> POVWidgetClass;

	UPROPERTY(EditAnywhere, Category = "GhostChase")
	FLinearColor TargetHighlightColor = FLinearColor::Red; // 강조 색상
};
