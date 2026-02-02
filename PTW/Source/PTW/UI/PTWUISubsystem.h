// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/LocalPlayerSubsystem.h"
#include "PTWUISubsystem.generated.h"

class UAbilitySystemComponent;
class UUserWidget;
class UPTWInGameHUD;
class UPTWDamageIndicator;

UENUM(BlueprintType)
enum class EUIInputPolicy : uint8
{
	GameOnly,
	UIOnly,
	GameAndUI
};

USTRUCT()
struct FUIStackEntry
{
	GENERATED_BODY()

	UPROPERTY()
	TObjectPtr<UUserWidget> Widget = nullptr;

	UPROPERTY()
	int32 ZOrder = 0;

	UPROPERTY()
	EUIInputPolicy InputPolicy = EUIInputPolicy::UIOnly;
};

/**
 * 
 */
UCLASS()
class PTW_API UPTWUISubsystem : public ULocalPlayerSubsystem
{
	GENERATED_BODY()
	
public:
	/* ULocalPlayerSubsystem */
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;

	/* ASC 호출 */
	UFUNCTION(BlueprintCallable)
	UAbilitySystemComponent* GetLocalPlayerASC() const;

	/** UI 스택관리 */
	void PushWidget(TSubclassOf<UUserWidget> WidgetClass, EUIInputPolicy InputPolicy);
	void PopWidget();
	bool IsWidgetInStack(TSubclassOf<UUserWidget> WidgetClass) const;
	bool IsStackEmpty() const;

	/** HUD */
	void ShowHUD(TSubclassOf<UUserWidget> HUDClass);

	/* 상시 존재 UI 생성 (랭킹보드) */
	UUserWidget* CreatePersistentWidget(TSubclassOf<UUserWidget> WidgetClass, int32 ZOrder = 10);
	/* 위젯 가시성 조절 */
	void SetWidgetVisibility(TSubclassOf<UUserWidget> WidgetClass, bool bVisible);

	/* 데미지 인디케이터 */
	UFUNCTION()
	void ShowDamageIndicator(const FVector& DamageCauserLocation);
	void SetDamageIndicatorClass(TSubclassOf<UPTWDamageIndicator> InClass) { DamageIndicatorClass = InClass; }

private:
	/** Helpers */
	UUserWidget* GetOrCreateWidget(TSubclassOf<UUserWidget> WidgetClass);
	APlayerController* GetPlayerController() const;

	void ApplyInputPolicy(EUIInputPolicy Policy);

	/** Stack-based UI */
	UPROPERTY()
	TArray<FUIStackEntry> WidgetStack;

	/** Cached widgets */
	UPROPERTY()
	TMap<TSubclassOf<UUserWidget>, TObjectPtr<UUserWidget>> CachedWidgets;

	// 스택 외 독립적으로 화면에 떠 있는 위젯들 (Key: 클래스, Value: 위젯 인스턴스)
	UPROPERTY()
	TMap<TSubclassOf<UUserWidget>, TObjectPtr<UUserWidget>> PersistentWidgets;

	/** HUD */
	UPROPERTY()
	TObjectPtr<UUserWidget> HUDWidget = nullptr;

	/* 데미지 인디케이터 */
	UPROPERTY(EditDefaultsOnly, Category = "DamageIndicator")
	TSubclassOf<UPTWDamageIndicator> DamageIndicatorClass;
};
