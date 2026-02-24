// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Gameplay/PTWPickupItemBase.h"
#include "PTWPickupWeapon.generated.h"

class UPTWWeaponInstance;
class UPTWItemInstance;

UCLASS()
class PTW_API APTWPickupWeapon : public APTWPickupItemBase
{
	GENERATED_BODY()

public:
	// Sets default values for this actor's properties
	APTWPickupWeapon();
	FORCEINLINE void SetWeaponInstance(UPTWWeaponInstance* Inst) {WeaponInstance = Inst;}
	virtual bool ReplicateSubobjects(class UActorChannel* Channel, class FOutBunch* Bunch, FReplicationFlags* RepFlags) override;
	virtual void GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const override;
	
protected:
	virtual void BeginPlay() override;
	virtual void OnPickedUp(class APTWPlayerCharacter* Player);

protected:
	UPROPERTY(VisibleAnywhere, Replicated)
	TObjectPtr<UPTWWeaponInstance> WeaponInstance;
};
