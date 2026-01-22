// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "PTWInventoryComponent.generated.h"


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
	
protected:
	// Called when the game starts
	virtual void BeginPlay() override;
	
private:
	
	
};
