// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "PTWGhostChaseControllerComponent.generated.h"

class UWidgetComponent;
class UUserWidget;

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

	/* 이름 강조 */
	void ApplyNameTagHighlight(APawn* TargetPawn, UWidgetComponent* WidgetComp);

protected:
	UPROPERTY(Replicated)
	TObjectPtr<APawn> CurrentTargetPawn;
		
	UPROPERTY(EditAnywhere, Category = "GhostChase")
	FLinearColor TargetHighlightColor = FLinearColor::Red; // 강조 색상
};
