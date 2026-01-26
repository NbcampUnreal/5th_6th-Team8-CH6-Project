// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "PTWInventoryComponent.generated.h"


class UPTWItemInstance;
class APTWWeaponActor;
class UPTWItemDefinition;

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class PTW_API UPTWInventoryComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	// Sets default values for this component's properties
	UPTWInventoryComponent();
	
	//FIXME : 파리미터 임시 추가(WeaponActor)
	void AddItem(TObjectPtr<UPTWItemDefinition> ItemClass, APTWWeaponActor* WeaponActor);
	void SwapWeapon(int32 SlotIndex);
	
	UFUNCTION(BlueprintCallable)
	void EqiupWeapon(int32 SlotIndex);
	
	UFUNCTION(BlueprintPure)
	FORCEINLINE UPTWItemInstance* GetCurrentWeaponActor() {return CurrentWeapon;}
	
	virtual void GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const override;
	virtual bool ReplicateSubobjects(class UActorChannel* Channel, class FOutBunch* Bunch, FReplicationFlags* RepFlags) override;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Replicated)
	TObjectPtr<UPTWItemInstance> CurrentWeapon;
	
protected:
	// Called when the game starts
	virtual void BeginPlay() override;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly,Replicated)
	TArray<TObjectPtr<UPTWItemInstance>> WeaponArr;
	
	UFUNCTION(BlueprintCallable, Server, Reliable, WithValidation)
	void Server_EquipWeapon(int32 SlotIndex);
	
	UFUNCTION(Client, Reliable, WithValidation)
	void ServerRPCSetCurrentWeapon(int32 SlotIndex);

private:
	//FIXME : 일단 임시로 현재 무기 Actor로 저장
	//TObjectPtr<UPTWItemInstance> CurrentWeapon;
};
