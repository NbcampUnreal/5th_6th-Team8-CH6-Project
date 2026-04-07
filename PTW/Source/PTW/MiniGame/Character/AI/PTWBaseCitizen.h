// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "CoreFramework/PTWBaseCharacter.h"
#include "CoreFramework/Interface/PTWActorInfoInterface.h"
#include "PTWBaseCitizen.generated.h"

UCLASS()
class PTW_API APTWBaseCitizen : public APTWBaseCharacter, public IPTWActorInfoInterface
{
	GENERATED_BODY()

public:
	// Sets default values for this character's properties
	APTWBaseCitizen();
	
	virtual FText GetDisplayName_Implementation() const override { return DisplayName; };
protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ActorInfo")
	FText DisplayName;
public:
	// Called every frame
	virtual void Tick(float DeltaTime) override;
	
	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;
};
