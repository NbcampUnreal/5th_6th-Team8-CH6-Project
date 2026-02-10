// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "PTWItemSlot.generated.h"

class UImage;
class UPTWItemDefinition;
/**
 * 
 */
UCLASS()
class PTW_API UPTWItemSlot : public UUserWidget
{
	GENERATED_BODY()
	
public:
	void SetupSlot(const UPTWItemDefinition* ItemDef);

protected:
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UImage> ItemIcon;
};
