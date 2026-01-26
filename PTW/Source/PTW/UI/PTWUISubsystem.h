// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/LocalPlayerSubsystem.h"
#include "PTWUISubsystem.generated.h"

class UUserWidget;

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

	/** Window Stack */
	void PushWidget(TSubclassOf<UUserWidget> WidgetClass);
	void PopWidget();
	bool IsWidgetInStack(TSubclassOf<UUserWidget> WidgetClass) const;

	/** HUD */
	void ShowHUD(TSubclassOf<UUserWidget> HUDClass);

private:
	/** Stack-based UI */
	UPROPERTY()
	TArray<FUIStackEntry> WidgetStack;

	/** Cached widgets */
	UPROPERTY()
	TMap<TSubclassOf<UUserWidget>, TObjectPtr<UUserWidget>> CachedWidgets;

	/** HUD */
	UPROPERTY()
	TObjectPtr<UUserWidget> HUDWidget = nullptr;

	/** Helpers */
	UUserWidget* GetOrCreateWidget(TSubclassOf<UUserWidget> WidgetClass);
	APlayerController* GetPlayerController() const;

	void ApplyInputPolicy(EUIInputPolicy Policy);
};
