// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "PTWInventoryComponent.generated.h"


//class UPTWItemInstance;
class APTWWeaponActor;
class UPTWItemDefinition;

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class PTW_API UPTWInventoryComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	// Sets default values for this component's properties
	UPTWInventoryComponent();
	
	void AddItem(const UPTWItemDefinition& AddItemDef);
	void SwapWeapon(int32 SlotIndex);
	
	UFUNCTION(BlueprintPure)
	FORCEINLINE APTWWeaponActor* GetCurrentWeaponActor() {return CurrentWeapon;}
	
	UFUNCTION(BlueprintCallable)
	FORCEINLINE void SetCurrentWeapon(APTWWeaponActor* SetWeaponActor) { CurrentWeapon = SetWeaponActor; }
protected:
	// Called when the game starts
	virtual void BeginPlay() override;
	
private:
	//TArray<UPTWItemInstance> WeaponArr;
	
	//FIXME : 일단 임시로 현재 무기 Actor로 저장
	TObjectPtr<APTWWeaponActor> CurrentWeapon;
};
