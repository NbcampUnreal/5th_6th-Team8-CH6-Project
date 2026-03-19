// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "PTWCARControllerComponent.generated.h"

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class PTW_API UPTWCARControllerComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UPTWCARControllerComponent();

protected:
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	
public:
	virtual void InitializeController();
	void DestroyOtherTeamNameTag();
	void SetTeamId(int32 NewTeamId);	// ServerOnly Invoke
	
	UFUNCTION()
	void OnRep_TeamId();
	
protected:
	UPROPERTY(ReplicatedUsing=OnRep_TeamId)
	int32 TeamId = -1;
};
