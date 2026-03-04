// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "PTWAttributeSet.h"
#include "PTWDeliveryAttributeSet.generated.h"

/**
 * 
 */

#define ATTRIBUTE_ACCESSORS(ClassName, PropertyName) \
GAMEPLAYATTRIBUTE_PROPERTY_GETTER(ClassName, PropertyName) \
GAMEPLAYATTRIBUTE_VALUE_GETTER(PropertyName) \
GAMEPLAYATTRIBUTE_VALUE_SETTER(PropertyName) \
GAMEPLAYATTRIBUTE_VALUE_INITTER(PropertyName)


UCLASS()
class PTW_API UPTWDeliveryAttributeSet : public UPTWAttributeSet
{
	GENERATED_BODY()
public:
	UPTWDeliveryAttributeSet();
	
	virtual void GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const override;
	virtual void PreAttributeChange(const FGameplayAttribute& Attribute, float& NewValue) override;
	virtual void PostGameplayEffectExecute(const FGameplayEffectModCallbackData& Data) override;
	virtual void PostAttributeChange(const FGameplayAttribute& Attribute, float OldValue, float NewValue) override;
	
	UPROPERTY(BlueprintReadOnly, ReplicatedUsing = OnRep_BatteryLevel, Category = "Delivery|Attributes")
	FGameplayAttributeData BatteryLevel;
	ATTRIBUTE_ACCESSORS(UPTWDeliveryAttributeSet, BatteryLevel);
	
	UPROPERTY(BlueprintReadOnly, ReplicatedUsing = OnRep_MoveSpeedModifier, Category = "Delivery|Attributes")
	FGameplayAttributeData MoveSpeedModifier;
	ATTRIBUTE_ACCESSORS(UPTWDeliveryAttributeSet, MoveSpeedModifier);
	
protected:
	UFUNCTION()
	void OnRep_BatteryLevel(const FGameplayAttributeData& OldBatteryLevel);
	
	UFUNCTION()
	void OnRep_MoveSpeedModifier(const FGameplayAttributeData& OldMoveSpeedMod);
	
	void UpdateCharacterSpeed();
	
protected:
	float BaseSpeed;
};
