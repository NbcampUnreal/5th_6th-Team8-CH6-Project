// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "PTWResultCharacter.generated.h"

class UWidgetComponent;

UCLASS()
class PTW_API APTWResultCharacter : public AActor
{
	GENERATED_BODY()
	
public:
	APTWResultCharacter();

	void InitializeResult(bool bIsWinner, const FString& InPlayerName);

protected:
	UFUNCTION(NetMulticast, Reliable)
	void Multicast_SetResultState(bool bIsWinner, const FString& InPlayerName);

protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	TObjectPtr<USceneComponent> Scene;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	TObjectPtr<USkeletalMeshComponent> MeshComp;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "UI")
	TObjectPtr<UWidgetComponent> NameTagWidget;

	UPROPERTY(EditDefaultsOnly, Category = "Result|Winner")
	TObjectPtr<UAnimMontage> WinMontage;
	UPROPERTY(EditDefaultsOnly, Category = "Result|Loser")
	TObjectPtr<UAnimMontage> LoseMontage;
};
