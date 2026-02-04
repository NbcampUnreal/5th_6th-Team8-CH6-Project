// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "PTWLobbyPortal.generated.h"

class USphereComponent;

UCLASS()
class PTW_API APTWLobbyPortal : public AActor
{
	GENERATED_BODY()
	
public:	
	
	APTWLobbyPortal();

	void SetPortalEnabled(bool bEnable);
	
protected:
	virtual void BeginPlay() override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	
	UFUNCTION()
	void OnComponentBeginOverlap(
		UPrimitiveComponent* OverlappedComponent,
		AActor* OtherActor,
		UPrimitiveComponent* OtherComp,
		int32 OtherBodyIndex,
		bool bFromSweep,
		const FHitResult& SweepResult
	);
	
	UFUNCTION()
	void OnComponentEndOverlap(UPrimitiveComponent* OverlappedComponent,
		AActor* OtherActor,
		UPrimitiveComponent* OtherComp,
		int32 OtherBodyIndex
		);
	
	void UpdatePortalCount();
	void ApplyPortalEnabled(bool bEnable);
	
	
	UPROPERTY(VisibleAnywhere, Category = "Component");
	TObjectPtr<USphereComponent> SphereCollision;

	TSet<TObjectPtr<APlayerState>> PlayerInPortal;

	UPROPERTY(ReplicatedUsing=OnRep_PortalEnabled)
	bool bPortalEnabled;

	UFUNCTION()
	void OnRep_PortalEnabled();

	
};
