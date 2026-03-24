// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "StartBlockActor.generated.h"

UCLASS()
class PTW_API AStartBlockActor : public AActor
{
	GENERATED_BODY()

public:
	AStartBlockActor();
	void HideActor();
	virtual void GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const override;

protected:
	virtual void BeginPlay() override;
	
	UFUNCTION()
	void OnRep_IsHidden();
	
protected:
	UPROPERTY(EditAnywhere)
	TObjectPtr<USceneComponent> SceneComponent;

	UPROPERTY(EditAnywhere)
	TObjectPtr<UStaticMeshComponent> StaticMeshComponent;
	
	UPROPERTY(ReplicatedUsing = OnRep_IsHidden)
	bool bIsHidden;
	
};
