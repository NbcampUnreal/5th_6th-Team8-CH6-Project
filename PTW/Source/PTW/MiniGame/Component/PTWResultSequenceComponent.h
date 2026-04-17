// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "PTWGameModeBaseComponent.h"
#include "Components/ActorComponent.h"
#include "PTWResultSequenceComponent.generated.h"


class APTWPlayerState;
class APTWResultCharacter;

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class PTW_API UPTWResultSequenceComponent : public UPTWGameModeBaseComponent
{
	GENERATED_BODY()

public:	
	UPTWResultSequenceComponent();
	
	void StartResultSequence();
protected:
	virtual void BeginPlay() override;

private:
	
	void FinishEndGameSequence();
	
	bool IsWinner(APTWPlayerState* PlayerState);

	
private:
	UPROPERTY(EditDefaultsOnly, Category = "Rule|Result")
	TSubclassOf<APTWResultCharacter> ResultCharacterClass;
	UPROPERTY(EditDefaultsOnly, Category = "Rule|Result")
	float ResultSequenceDuration = 15.0f;

	FTimerHandle ResultTimerHandle;
};
