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

protected:
	virtual void BeginPlay() override;
	
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
	
	UPROPERTY(VisibleAnywhere, Category = "Component");
	TObjectPtr<USphereComponent> SphereCollision;

	TSet<TObjectPtr<APlayerState>> PlayerInPortal;
};
