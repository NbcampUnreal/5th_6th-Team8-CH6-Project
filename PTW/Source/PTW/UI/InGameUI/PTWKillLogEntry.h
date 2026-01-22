// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "PTWKillLogEntry.generated.h"

DECLARE_MULTICAST_DELEGATE_OneParam(FOnKillLogExpired, class UPTWKillLogEntry*);

class UTextBlock;

/**
 * 
 */
UCLASS()
class PTW_API UPTWKillLogEntry : public UUserWidget
{
	GENERATED_BODY()
	
public:
	void Init(
		const FString& Killer,
		const FString& Victim,
		float LifeTime); // 인자에 무기 종류 추가해야함

	/* 만료 이벤트 */
	FOnKillLogExpired OnExpired;

protected:
	void HandleExpired();

	/* 킬 텍스트 */
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> KillText;

	FTimerHandle LifeTimerHandle;
};
