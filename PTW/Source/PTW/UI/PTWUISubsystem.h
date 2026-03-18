// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/LocalPlayerSubsystem.h"
#include "UI/InGameUI/PTWNotificationWidget.h"
#include "PTWUISubsystem.generated.h"

class UAbilitySystemComponent;
class UUserWidget;
class UPTWInGameHUD;
class UPTWDamageIndicator;
class UPTWChatInput;
class UPTWChatList;
class UPTWNotificationWidget;

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

	/* UI 스택관리 */
	void PushWidget(TSubclassOf<UUserWidget> WidgetClass, EUIInputPolicy InputPolicy);
	void PopWidget();
	bool IsWidgetInStack(TSubclassOf<UUserWidget> WidgetClass) const;
	bool IsStackEmpty() const;
	void StackReset();
	UUserWidget* GetTopWidget() const;

	/* HUD */
	void ShowHUD(TSubclassOf<UUserWidget> HUDClass);

	/* 상시 존재 UI 생성 (랭킹보드) */
	UUserWidget* CreatePersistentWidget(TSubclassOf<UUserWidget> WidgetClass, int32 ZOrder = 10);

	/* 임의로 지울수 없는 1회성 UI들 */
	UUserWidget* ShowSystemWidget(TSubclassOf<UUserWidget> WidgetClass, int32 ZOrder = 60);
	void HideSystemWidget(TSubclassOf<UUserWidget> WidgetClass);

	/* 위젯 가시성 조절 */
	void SetWidgetVisibility(TSubclassOf<UUserWidget> WidgetClass, bool bVisible);

	/* 데미지 인디케이터 */
	UFUNCTION()
	void ShowDamageIndicator(const FVector& DamageCauserLocation);
	void SetDamageIndicatorClass(TSubclassOf<UPTWDamageIndicator> InClass) { DamageIndicatorClass = InClass; }

	/* 채팅 */
	void SetChatInputClass(TSubclassOf<UPTWChatInput> InClass) { ChatInputClass = InClass; }
	void SetChatListClass(TSubclassOf<UPTWChatList> InClass) { ChatListClass = InClass; }

	/* Helpers */
	UUserWidget* GetOrCreateWidget(TSubclassOf<UUserWidget> WidgetClass);

	/* 입력정책 기본값 수정 */
	void SetDefaultInputPolicy(EUIInputPolicy NewPolicy) { DefaultInputPolicy = NewPolicy; }

	/* 알림 위젯 */
	void PushNotification(const FNotificationData& Data);

private:
	/* Helpers */
	APlayerController* GetPlayerController() const;

	void ApplyInputPolicy(EUIInputPolicy Policy);

	void TryInitializeHUDASC();

	/* 입력정책 기본값 저장 */
	UPROPERTY()
	EUIInputPolicy DefaultInputPolicy = EUIInputPolicy::GameOnly;

	/* UI 스택 */
	UPROPERTY()
	TArray<FUIStackEntry> WidgetStack;

	/* 캐시된 위젯 */
	UPROPERTY()
	TMap<TSubclassOf<UUserWidget>, TObjectPtr<UUserWidget>> CachedWidgets;

	/** HUD */
	UPROPERTY()
	TObjectPtr<UUserWidget> HUDWidget = nullptr;

	/* 데미지 인디케이터 */
	UPROPERTY(EditDefaultsOnly, Category = "DamageIndicator")
	TSubclassOf<UPTWDamageIndicator> DamageIndicatorClass;

	/* 채팅 */
	UPROPERTY()
	TSubclassOf<UPTWChatInput> ChatInputClass;
	UPROPERTY()
	TSubclassOf<UPTWChatList> ChatListClass;
	
	/* ASC get 타이머 */
	FTimerHandle ASCInitTimerHandle;

	UPROPERTY(EditDefaultsOnly, Category = "UI")
	TSubclassOf<UUserWidget> PopupWidgetClass;
};
