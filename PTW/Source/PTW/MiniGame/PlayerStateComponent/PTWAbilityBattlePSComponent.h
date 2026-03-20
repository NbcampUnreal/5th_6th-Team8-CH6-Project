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

	virtual void GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const override;
	
	void AddDraftCharges();
	void DecreaseDraftCharges();
	
	void SetCurrentDraft(const TArray<FName>& NewDraft);
	void ResetCurrentDraft();
	
	UPROPERTY(Replicated,VisibleAnywhere)
	int32 DraftCharges = 1;

	UPROPERTY(VisibleAnywhere)
	TArray<FName> CurrentDraft;

	UPROPERTY(Replicated)
	bool bFirstDraftCompleted = false;

	FORCEINLINE TArray<FName> GetCurrentDraft() {return CurrentDraft;}
	
};
