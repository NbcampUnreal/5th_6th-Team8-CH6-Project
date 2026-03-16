// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "PTWItemInstance.h"
#include "Inventory/PTWItemDefinition.h"
#include "PTWActiveItemInstance.generated.h"

DECLARE_MULTICAST_DELEGATE_OneParam(FOnCurrentCountChanged, int32);
DECLARE_MULTICAST_DELEGATE(FOnItemDepleted);
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
	FORCEINLINE void SetCurrentCount(){ CurrentCount =  CurrentCount == 0 ? ItemDef->MaxUsage - 1 : CurrentCount; }
	FORCEINLINE int32 GetCurrentCount() const { return CurrentCount; }

	UFUNCTION()
	void OnRep_CurrentCount();

public:
	FOnCurrentCountChanged OnCurrentCountChanged;
	FOnItemDepleted OnItemDepleted;

protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Replicated, Category = "ItemDefault")
	int32 CurrentCount;
};
