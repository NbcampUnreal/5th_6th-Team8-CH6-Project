// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AttributeSet.h"
#include "AbilitySystemComponent.h"
#include "PTWBombAttributeSet.generated.h"

class APTWBombActor;

#define ATTRIBUTE_ACCESSORS(ClassName, PropertyName) \
GAMEPLAYATTRIBUTE_PROPERTY_GETTER(ClassName, PropertyName) \
GAMEPLAYATTRIBUTE_VALUE_GETTER(PropertyName) \
GAMEPLAYATTRIBUTE_VALUE_SETTER(PropertyName) \
GAMEPLAYATTRIBUTE_VALUE_INITTER(PropertyName)

UCLASS()
class PTW_API UPTWBombAttributeSet : public UAttributeSet
{
	GENERATED_BODY()

public:
	UPTWBombAttributeSet();

	/** 남은 시간(초) */
	UPROPERTY(BlueprintReadOnly, Category="Bomb", ReplicatedUsing=OnRep_RemainingTime)
	FGameplayAttributeData RemainingTime;
	ATTRIBUTE_ACCESSORS(UPTWBombAttributeSet, RemainingTime)

protected:
	UFUNCTION()
	void OnRep_RemainingTime(const FGameplayAttributeData& OldValue);

public:
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	virtual void PreAttributeChange(const FGameplayAttribute& Attribute, float& NewValue) override;
	virtual void PostAttributeBaseChange(const FGameplayAttribute& Attribute, float OldValue, float NewValue) const override;
};

