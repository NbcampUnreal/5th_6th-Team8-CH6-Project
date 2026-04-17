// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "MiniGame/ControllerComponent/PTWBaseControllerComponent.h"
#include "PTWBombControllerComponent.generated.h"

class APTWBombActor;
class UPTWUISubsystem;
/**
 * 
 */
UCLASS(Blueprintable, ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class PTW_API UPTWBombControllerComponent : public UPTWBaseControllerComponent
{
	GENERATED_BODY()
	
public:
	/* BombActor 델리게이트 바인딩 */
	void BindBombDelegate(APTWBombActor* NewBomb);
	void UnBindBombDelegate();

protected:
	/* 경고UI 표시 관련 함수 */
	void HandleBombOwnerChanged(APawn* NewOwnerPawn);
	void ShowBombUI();
	void HideBombUI();

	// 폭탄 경고 위젯
	UPROPERTY(EditDefaultsOnly, Category = "UI|Bomb")
	TSubclassOf<class UPTWBombWarning> BombWarningWidgetClass;

private:
	UPROPERTY()
	TObjectPtr<UPTWUISubsystem> UISubsystem;
	UPROPERTY()
	TObjectPtr<APTWBombActor> CachedBombActor;

};
