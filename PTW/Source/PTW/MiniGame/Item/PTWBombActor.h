// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "AbilitySystemInterface.h"
#include "AbilitySystemComponent.h"
#include "GameplayTagContainer.h"
#include "PTWBombActor.generated.h"

class UAbilitySystemComponent;
class UStaticMeshComponent;
class USphereComponent;
class UGameplayEffect;
class UPTWBombAttributeSet;
class UGameplayAbility;

UCLASS()
class PTW_API APTWBombActor : public AActor, public IAbilitySystemInterface
{
	GENERATED_BODY()

public:
	APTWBombActor();

	virtual UAbilitySystemComponent* GetAbilitySystemComponent() const override;

protected:
	virtual void BeginPlay() override;

	/** GAS */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="GAS")
	TObjectPtr<UAbilitySystemComponent> AbilitySystemComponent;

	UPROPERTY()
	TObjectPtr<UPTWBombAttributeSet> BombAttributeSet;

	/** Components */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Components")
	TObjectPtr<UStaticMeshComponent> BombMesh;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Components")
	TObjectPtr<USphereComponent> Collision;

	/** 폭탄 상태 태그  */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="GAS")
	FGameplayTag BombStateTag;

	/** Timer Effects */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Bomb|GAS")
	TSubclassOf<UGameplayEffect> SetTimeEffectClass;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Bomb|GAS")
	TSubclassOf<UGameplayEffect> CountdownEffectClass;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Bomb|GAS")
	TSubclassOf<UGameplayAbility> ExplodeAbilityClass;

public:
	UFUNCTION(BlueprintCallable, Category="Bomb")
	void RequestExplode(AActor* InstigatorActor);
	
	UFUNCTION(Server, Reliable)
	void ServerRequestExplode(AActor* InstigatorActor);

private:
	void HandleRemainingTimeChanged(const FOnAttributeChangeData& Data);

	FDelegateHandle RemainingTimeChangedHandle;

	void ApplyEffectToSelf(TSubclassOf<UGameplayEffect> EffectClass);

	/** ✅ 내부: GameplayEvent 발사 (Event.Bomb.Explode) */
	void SendExplodeEvent(AActor* InstigatorActor);

	/** (선택) 중복 폭발 방지용 */
	bool bExplodeRequested = false;
};
