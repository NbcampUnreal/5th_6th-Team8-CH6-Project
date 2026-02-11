// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "PTWItemInstance.h"
#include "Inventory/PTWItemDefinition.h"
#include "PTWActiveItemInstance.generated.h"

/**
 * 
 */
UCLASS()
class PTW_API UPTWActiveItemInstance : public UPTWItemInstance
{
	GENERATED_BODY()
	
public:
	virtual void GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const override;
	bool UsingActiveItem();
	FORCEINLINE void SetCurrentCount(){CurrentCount  = ItemDef->MaxUsage - 1;}
	
protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Replicated, Category = "ItemDefault")
	int32 CurrentCount;
};
