// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "AbilitySystemInterface.h"
#include "GameplayTagContainer.h"
#include "PTWBaseCharacter.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnCharacterDeathSignature, AActor*, DeadActor, AActor*, KillerActor);

class UAbilitySystemComponent;
class UAttributeSet;
class UGameplayAbility;
class UGameplayEffect;
class UPTWReactorComponent;


UCLASS()
class PTW_API APTWBaseCharacter : public ACharacter, public IAbilitySystemInterface
{
	GENERATED_BODY()


public:
	// 1. 생성자 (Constructor)
	APTWBaseCharacter();


	// 2. [Public] 인터페이스 함수 (외부에서 호출하는 함수)
	UFUNCTION(BlueprintPure)
	bool IsDead() const;

	virtual UAbilitySystemComponent* GetAbilitySystemComponent() const override;

	/*  ReactorComponet에서 Death 로직 구현됨 */
	virtual void HandleDeath(AActor* Attacker);


	// 3. [Public] Getter / Setter (FORCEINLINE 권장)
	FORCEINLINE UPTWReactorComponent* GetReactorComponent() const { return ReactorComponent; }

protected:
	//4. LifeCycle 함수
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

	// 5. [Protected] 내부 구현 로직 (상속받은 자식이 쓸 수 있는 함수)
	virtual void InitAbilityActorInfo();
	void GiveDefaultAbilities();
	void ApplyDefaultEffects();

private:
	// 6. [Private] 내부 전용 유틸리티 함수 (외부/자식 노출 X)

public:
	// 7. [Public] 멤버 변수 (대부분의 설정값)
	FGameplayTag DeadTag;

protected:
	// 8. [Protected] 멤버 변수 (내부 상태값)
	UPROPERTY(EditAnywhere, Category = "GAS|Default")
	TArray<TSubclassOf<UGameplayAbility>> DefaultAbilities;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "GAS|Default")
	TArray<TSubclassOf<UGameplayEffect>> DefaultEffects;
	UPROPERTY(Transient)
	TArray<TSubclassOf<UGameplayAbility>> AdditionalAbilities;
	UPROPERTY(Transient)
	TArray<TSubclassOf<UGameplayEffect>> AdditionalEffects;
	UPROPERTY()
	TObjectPtr<UAttributeSet> AttributeSet;

	// 9. [Protected] 컴포넌트 (Components)
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "GAS")
	TObjectPtr<UAbilitySystemComponent> AbilitySystemComponent;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<UPTWReactorComponent> ReactorComponent;

private:
	// 10. [Private] 멤버 변수 (완벽히 숨겨야 하는 값)
	
public:
	// 11. [Delegate] 델리게이트 (최하단 배치 규칙 준수)
	UPROPERTY(BlueprintAssignable)
	FOnCharacterDeathSignature OnCharacterDied;

};
