// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "PTWBaseCharacter.h"
#include "InputActionValue.h"
#include "PTWInputConfig.h"
#include "CoreFramework/PTWPlayerData.h"
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
	FORCEINLINE UInputMappingContext* GetDefaultMappingContext() const { return DefaultMappingContext; }

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
	FORCEINLINE bool GetStealthMode() const { return bIsStealth; }

protected:
	// 4. [Protected] 오버라이드 함수 (LifeCycle) - BeginPlay, EndPlay 등
	virtual void BeginPlay() override;
	virtual void PossessedBy(AController* NewController) override;
	virtual void OnRep_PlayerState() override;
	virtual void OnPlayerStateChanged(APlayerState* NewPlayerState, APlayerState* OldPlayerState) override;
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;
	virtual void InitAbilityActorInfo() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
	virtual void Landed(const FHitResult& Hit) override;
	virtual void HandleDeath(AActor* Attacker) override;
	virtual void Tick(float DeltaTime) override;
	
	// 5. [Protected] 내부 구현 로직 (상속받은 자식이 쓸 수 있는 함수)
	void Move(const FInputActionValue& Value);
	void Look(const FInputActionValue& Value);
	void Input_AbilityInputTagPressed(FGameplayTag InputTag);
	void Input_AbilityInputTagReleased(FGameplayTag InputTag);

	void InitCharacterState();

	void OnInputTriggered();
	void OnInputCompleted();
	void CheckIdleCondition();
	void SetIdleState(bool bNewState);
	
	/*인벤토리 관련 인풋 바인딩 함수(현정석(26.02.03))*/
	void EquipFirstWeapon(const FInputActionValue& Value);
	void EquipSecondWeapon(const FInputActionValue& Value);
	void UseActiveItem(const FInputActionValue& Value);

	/* 위젯에 닉네임 전달 */
	void UpdateNameTagText();
	
	/*인벤토리 RPC 함수 추가(현정석(26.02.03))*/
	UFUNCTION(Server, Reliable)
	void ServerRPCEquipWeapon(int32 SelectIndex); // 후에 인풋 바인딩에 무기 슬롯 인덱스 받을 수도?
	
	UFUNCTION(Server, Reliable)
	void ServerRPCUseActiveItem();
	
	/*움직임 제한 이벤트 함수(태그 적용 시 자동으로 움직임을 멈춤) 현정석(26.02.12)*/
	void RegisterGameplayTagEvents();
	UFUNCTION()
	void OnStasisTagChanged(const FGameplayTag Tag, int32 NewCount);
	
	/*StealthMode 관련*/
	UFUNCTION()
	void OnRep_StealthMode();

private:
	// 6. [Private] 내부 전용 유틸리티 함수 (외부/자식 노출 X)


public:
	// 7. [Public] 멤버 변수 (대부분의 설정값)
	UPROPERTY(Replicated, meta=(COND_SkipOwner))
	float AimPitch = 0.0f;
	
	UFUNCTION(Server, Reliable)
	void ServerRPCUpdateAimPitch(float NewAimPitch);
	// StealthMode 관련 함수 추가
	void SetStealthMode(bool bSetStealthMode);

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
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
	TObjectPtr<UInputAction> EquipFirstWeaponAction;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
	TObjectPtr<UInputAction> EquipSecondWeaponAction;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
	TObjectPtr<UInputAction> UseActiveItemAction;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
	TObjectPtr<UInputAction> DropItemAction;
	UPROPERTY(EditDefaultsOnly, Category = "Sound")
	TObjectPtr<USoundBase> LandSound;
	//FIXME : 테스트 용도
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Temp")
	TObjectPtr<UPTWItemDefinition> ItemDef;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "UI")
	TObjectPtr<UWidgetComponent> NameTagWidget;

	FTimerHandle NameTagRetryTimer;
	FTimerHandle IdleCheckTimerHandle;

	bool bIsAbilitiesInitialized = false;
	bool bIsIdleState = false;



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

	
	//Stealth 모드 전용(현정석)
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Components", ReplicatedUsing = OnRep_StealthMode)
	bool bIsStealth;
private:
	// 10. [Private] 멤버 변수 (완벽히 숨겨야 하는 값)


public:
	// 11. [Delegate] 델리게이트 (최하단 배치 규칙 준수)
	UFUNCTION()
	void OnPlayerDataLoaded(const FPTWPlayerData& NewData);

	bool bHasGivenStartupItems = false;
};
