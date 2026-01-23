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
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	TObjectPtr<UPTWItemInstance> CurrentWeapon;
	
protected:
	// Called when the game starts
	virtual void BeginPlay() override;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	TArray<UPTWItemInstance*> WeaponArr;
	

	
private:
	//FIXME : 일단 임시로 현재 무기 Actor로 저장
	//TObjectPtr<UPTWItemInstance> CurrentWeapon;
};
