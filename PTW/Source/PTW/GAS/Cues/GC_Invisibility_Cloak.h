// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameplayCueNotify_Actor.h"
#include "GC_Invisibility_Cloak.generated.h"

class APTWPlayerCharacter;
class UPTWItemInstance;

UCLASS()
class PTW_API AGC_Invisibility_Cloak : public AGameplayCueNotify_Actor
{
	GENERATED_BODY()

public:
	// Sets default values for this actor's properties
	AGC_Invisibility_Cloak();
	
	virtual bool WhileActive_Implementation(AActor* MyTarget, const FGameplayCueParameters& Parameters) override;
	virtual bool OnRemove_Implementation(AActor* MyTarget, const FGameplayCueParameters& Parameters) override;
	
protected:
	bool SetPlayer3PMesh(bool bIsHidden);
	
	void SetPlayer3PWeaponMesh(bool bIsHidden);
	
protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Source")
	TObjectPtr<APTWPlayerCharacter> OwnerCharacter;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Source")
	TObjectPtr<UPTWItemInstance> ItemInstnace;
	
	
};
