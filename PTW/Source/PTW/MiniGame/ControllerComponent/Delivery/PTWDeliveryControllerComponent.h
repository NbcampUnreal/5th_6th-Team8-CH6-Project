// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "MiniGame/ControllerComponent/PTWBaseControllerComponent.h"
#include "PTWDeliveryControllerComponent.generated.h"

class UPostProcessComponent;
class UPTWDeliveryHUD;
class UPTWBatterLevelWidget;

UCLASS(Blueprintable, ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class PTW_API UPTWDeliveryControllerComponent : public UPTWBaseControllerComponent
{
	GENERATED_BODY()

public:
	// Sets default values for this component's properties
	UPTWDeliveryControllerComponent();
	void AddBatteryUI();
	void ShowCountDownWidget();
	void SetCountDownText(int32 Count);
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	FORCEINLINE void SetRank(int32 Rank) {MyCurrentRank = Rank;};
	FORCEINLINE void SetTraveledDistance(float NewTraveledDistance) { TraveledDistance = NewTraveledDistance;}
	FORCEINLINE float GetTraveledDistance() const { return TraveledDistance;}
	
	UFUNCTION()
	void RaceRankUpdate();
	
public:
	UPROPERTY(ReplicatedUsing = OnRep_CurrentRank)
	int32 MyCurrentRank = 0;
	
protected:
	virtual void BeginPlay() override;
	
	UFUNCTION(Client, Reliable)
	void ClientRPC_AddBatteryUI();
	
	UFUNCTION(Client, Reliable)
	void ClientRPC_ShowCountDownWidget();
	
	UFUNCTION(Client, Reliable)
	void ClientRPC_SetCountDownText(int32 Count);
	
	UFUNCTION()
	void OnRep_CurrentRank(int32 OldRank);
	
	void UpdateVignette(float DeltaTime);
	
	virtual void TickComponent(float DeltaTime, enum ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

protected:
	UPROPERTY(EditAnywhere)
	TSubclassOf<UUserWidget> DeliveryHUDClass;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	TObjectPtr<UPTWDeliveryHUD> DeliveryHUDWidgetInstance;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PostProcess")
	TObjectPtr<APostProcessVolume> PostProcessComp;
	
	UPROPERTY()
	float TraveledDistance = 0.0f;
};
