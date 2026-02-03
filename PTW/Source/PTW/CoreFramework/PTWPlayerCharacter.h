// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "PTWBaseCharacter.h"
#include "InputActionValue.h"
#include "PTWInputConfig.h"
#include "PTWPlayerCharacter.generated.h"

class APTWPlayerState;
class UPTWItemDefinition;
class UCameraComponent;
class UInputMappingContext;
class UInputAction;
class UPTWInventoryComponent;
class UWidgetComponent;
class UPTWWeaponComponent;
class UPTWReactorComponent;
class UPTWInteractComponent;

UCLASS()
class PTW_API APTWPlayerCharacter : public APTWBaseCharacter
{
	GENERATED_BODY()
	
public:
	// 1. 생성자 (Constructor)
	APTWPlayerCharacter();
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	// 2. [Public] 인터페이스 함수 (외부에서 호출하는 함수)


	// 3. [Public] Getter / Setter (FORCEINLINE 권장)
	FORCEINLINE UPTWWeaponComponent* GetWeaponComponent() const { return WeaponComponent; }
	FORCEINLINE UCameraComponent* GetPlayerCamera() const { return PlayerCamera; }
	FORCEINLINE UPTWInventoryComponent* GetInventoryComponent() const { return InventoryComponent; }
	FORCEINLINE UWidgetComponent* GetNameTagWidget() const { return NameTagWidget; }
	UFUNCTION(BlueprintPure, Category = "Mesh")
	FORCEINLINE USkeletalMeshComponent* GetMesh1P() const { return Mesh1P; }
	UFUNCTION(BlueprintPure, Category = "Mesh")
	FORCEINLINE USkeletalMeshComponent* GetMesh3P() const { return GetMesh(); }
	FORCEINLINE UPTWInteractComponent* GetInteractComponent() const { return InteractComponent; }

protected:
	// 4. [Protected] 오버라이드 함수 (LifeCycle) - BeginPlay, EndPlay 등
	virtual void BeginPlay() override;
	virtual void PossessedBy(AController* NewController) override;
	virtual void OnRep_PlayerState() override;
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;
	virtual void InitAbilityActorInfo() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;


	// 5. [Protected] 내부 구현 로직 (상속받은 자식이 쓸 수 있는 함수)
	void Move(const FInputActionValue& Value);
	void Look(const FInputActionValue& Value);
	void Input_AbilityInputTagPressed(FGameplayTag InputTag);
	void Input_AbilityInputTagReleased(FGameplayTag InputTag);

	/* 위젯에 닉네임 전달 */
	void UpdateNameTagText();

private:
	// 6. [Private] 내부 전용 유틸리티 함수 (외부/자식 노출 X)


public:
	// 7. [Public] 멤버 변수 (대부분의 설정값)


protected:
	// 8. [Protected] 멤버 변수 (내부 상태값)

	UPROPERTY(EditDefaultsOnly, Category = "Input")
	TObjectPtr<UPTWInputConfig> InputConfig;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
	TObjectPtr<UInputMappingContext> DefaultMappingContext;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
	TObjectPtr<UInputAction> MoveAction;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
	TObjectPtr<UInputAction> LookAction;

	//FIXME : 테스트 용도
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Temp")
	TObjectPtr<UPTWItemDefinition> ItemDef;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "UI")
	TObjectPtr<UWidgetComponent> NameTagWidget;

	FTimerHandle NameTagRetryTimer;


	// 9. [Protected] 컴포넌트 (Components)
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Camera")
	TObjectPtr<UCameraComponent> PlayerCamera;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Mesh, meta = (AllowPrivateAccess = "true"))
	TObjectPtr<USkeletalMeshComponent> Mesh1P;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Inventory", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UPTWInventoryComponent> InventoryComponent;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<UPTWWeaponComponent> WeaponComponent;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<UPTWInteractComponent> InteractComponent;

private:
	// 10. [Private] 멤버 변수 (완벽히 숨겨야 하는 값)


public:
	// 11. [Delegate] 델리게이트 (최하단 배치 규칙 준수)


};
