// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "PTWWeaponActor.generated.h"

class UPTWWeaponData;

UCLASS()
class PTW_API APTWWeaponActor : public AActor
{
	GENERATED_BODY()

public:
	APTWWeaponActor();
	
	UFUNCTION(BlueprintPure)
	FORCEINLINE USceneComponent* GetMuzzleComponent() const { return MuzzleSocket; }
	
	UFUNCTION(BlueprintPure)
	FORCEINLINE UPTWWeaponData* GetWeaponData() const {return WeaponData;}
	
	UFUNCTION(BlueprintPure)
	FORCEINLINE UStaticMeshComponent* GetStaticMeshComponent() const {return WeaponMesh;}
	
	void SetupVisualPerspective(bool bIs1P);
	
protected:
	virtual void BeginPlay() override;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Weapon")
	TObjectPtr<USceneComponent> RootScene;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Weapon")
	TObjectPtr<USceneComponent> MuzzleSocket;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Weapon")
	TObjectPtr<UStaticMeshComponent> WeaponMesh;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Weapon|Data")
	TObjectPtr<UPTWWeaponData> WeaponData;	
};
