// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "PTWAbilityBattlePSComponent.generated.h"



UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class PTW_API UPTWAbilityBattlePSComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UPTWAbilityBattlePSComponent();

	void AddDraftCharges();
	void DecreaseDraftCharges();
	
	void SetCurrentDraft(const TArray<FName>& NewDraft);
	void ResetCurrentDraft();
	
	UPROPERTY(VisibleAnywhere)
	int32 DraftCharges;

	UPROPERTY(VisibleAnywhere)
	TArray<FName> CurrentDraft;

	UPROPERTY()
	bool bFirstDraftCompleted;

	FORCEINLINE TArray<FName> GetCurrentDraft() {return CurrentDraft;}
	
};
