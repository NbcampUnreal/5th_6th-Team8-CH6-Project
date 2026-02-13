// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "AbilitySystemInterface.h"
#include "GameplayTagContainer.h"
#include "PTWBombActor.generated.h"

class UAbilitySystemComponent;
class UStaticMeshComponent;
class USphereComponent;
class UGameplayEffect;
class UPTWBombAttributeSet;
class UGameplayAbility;
class APawn;

DECLARE_MULTICAST_DELEGATE_OneParam(FOnBombTimeExpired, AActor* /*InstigatorActor*/);

UCLASS()
class PTW_API APTWBombActor : public AActor, public IAbilitySystemInterface
{
	GENERATED_BODY()

public:
	APTWBombActor();

	// IAbilitySystemInterface
	virtual UAbilitySystemComponent* GetAbilitySystemComponent() const override;

	// Replication
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	FOnBombTimeExpired OnBombTimeExpired;
	
protected:
	virtual void BeginPlay() override;

	/** Components */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Components")
	TObjectPtr<USphereComponent> Collision;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Components")
	TObjectPtr<UStaticMeshComponent> BombMesh;

	/** GAS */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="GAS")
	TObjectPtr<UAbilitySystemComponent> AbilitySystemComponent;

	UPROPERTY()
	TObjectPtr<UPTWBombAttributeSet> BombAttributeSet;

	/** 폭탄 상태 태그 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="GAS")
	FGameplayTag BombStateTag;

	/** Timer Effects */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Bomb|GAS")
	TSubclassOf<UGameplayEffect> SetTimeEffectClass;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Bomb|GAS")
	TSubclassOf<UGameplayEffect> CountdownEffectClass;


public:
	/** 폭발 요청 */
	UFUNCTION(BlueprintCallable, Category="Bomb")
	void RequestExplode(AActor* InstigatorActor);

	UFUNCTION(Server, Reliable)
	void ServerRequestExplode(AActor* InstigatorActor);
	
	UFUNCTION(NetMulticast, Reliable)
	void Multicast_PlayExplosionCue(const FVector& Loc, AActor* InstigatorActor);
	
	UFUNCTION(BlueprintCallable, Category="Bomb")
	void SetBombOwner(APawn* NewOwnerPawn);

	UFUNCTION(BlueprintPure, Category="Bomb")
	APawn* GetBombOwner() const { return BombOwnerPawn; }

	FORCEINLINE APawn* GetBombOwnerPawn() const { return BombOwnerPawn; }
	
	UFUNCTION(BlueprintPure, Category="Bomb")
	APlayerState* GetBombOwnerPlayerState() const
	{
		return BombOwnerPawn ? BombOwnerPawn->GetPlayerState() : nullptr;
	}

protected:
	UFUNCTION()
	void OnRep_BombOwnerPawn();

	void AttachToOwnerPawn();
	
	void ApplyEffectToSelf(TSubclassOf<UGameplayEffect> EffectClass);

	/** RemainingTime 변화 로그 */
	void HandleRemainingTimeChanged(const struct FOnAttributeChangeData& Data);
	
	bool ExplosionOverlapSetter(TArray<FOverlapResult>& OverlapResults);
	bool CheckingBlock(FHitResult& OutHit, const FVector ExplosionLocation, AActor* HitActor);
	void ApplyExplosionDamage(TArray<FOverlapResult>& OverlapResults, float FinalDamage, AActor* InstigatorActor);

	//연출효과 변수
	void UpdateBombEffects(float NewTime);
protected:
	/** 소유자 */
	UPROPERTY(ReplicatedUsing=OnRep_BombOwnerPawn)
	TObjectPtr<APawn> BombOwnerPawn;

	/** 중복 폭발 방지 */
	UPROPERTY()
	bool bExplodeRequested = false;
	
	UPROPERTY()
	bool bTimeExpiredNotified = false;

	/** 폭발 반경 */
	UPROPERTY(EditDefaultsOnly, Category="Bomb|Explosion")
	float ExplosionRad = 350.f;

	UPROPERTY(EditDefaultsOnly, Category="Bomb|Explosion")
	TEnumAsByte<ECollisionChannel> ExplosionChannel;

	/** 데미지 적용용 GE */
	UPROPERTY(EditDefaultsOnly, Category="Bomb|Explosion")
	TSubclassOf<UGameplayEffect> DamageEffectClass;

	UPROPERTY(EditDefaultsOnly, Category="Bomb|Explosion")
	FGameplayTag DamageSetByCallerTag;

	/** 폭발/피격 큐 */
	UPROPERTY(EditDefaultsOnly, Category="Bomb|Cue")
	FGameplayTag ExplosionCueTag;

	UPROPERTY(EditDefaultsOnly, Category="Bomb|Cue")
	FGameplayTag HitImpactCueTag;

	/** 폭탄 기본 데미지 */
	UPROPERTY(EditDefaultsOnly, Category="Bomb|Explosion")
	float BaseBombDamage = 999.f;

	//연출효과
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<UAudioComponent> AudioComponent;
	UPROPERTY(EditDefaultsOnly, Category = "Bomb|Audio")
	TArray<USoundBase*> CountdownSounds;
	UPROPERTY()
	TObjectPtr<UMaterialInstanceDynamic> BombDynamicMat;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<UAudioComponent> AudioLoopComponent;
	UPROPERTY(EditDefaultsOnly, Category = "Bomb|Audio")
	TObjectPtr<USoundBase> AudioLoopSound;

	int32 CurrentSoundPhaseIndex = -1;
};
