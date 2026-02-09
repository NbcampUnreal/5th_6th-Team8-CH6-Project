// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameplayEffectTypes.h"
#include "GameFramework/Actor.h"
#include "PTWProjectile.generated.h"

class UCapsuleComponent;
class UProjectileMovementComponent;

UCLASS()
class PTW_API APTWProjectile : public AActor
{
	GENERATED_BODY()

public:
	APTWProjectile();

	
public:
	UPROPERTY()
	FGameplayEffectSpecHandle DamageSpecHandle;

protected:
	virtual void BeginPlay() override;
	
	UFUNCTION()
	void OnHit(UPrimitiveComponent* HitComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit);
	
	bool ExplosionOverlapSetter(TArray<FOverlapResult>& OverlapResults);
	
	void ApplyExplosionDamage(TArray<FOverlapResult>& OverlapResults, float FinalDamage);
	
	bool CheckingBlock(FHitResult& ObstarcleHit, const FVector ExplosionLocation, const AActor* HitActor);
	
protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Projectile")
	TObjectPtr<UProjectileMovementComponent> ProjectileMovementComponent;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Component")
	TObjectPtr<UCapsuleComponent> CapsuleComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Component")
	TObjectPtr<UStaticMeshComponent> MeshComponent;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Explosion")
	float ExplosionRad;
	
	
};
