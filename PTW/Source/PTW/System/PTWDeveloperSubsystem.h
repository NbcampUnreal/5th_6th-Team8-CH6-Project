// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "Minigame/PTWMiniGameMapRow.h"
#include "PTWDeveloperSubsystem.generated.h"


UCLASS()
class PTW_API UPTWDeveloperSubsystem : public UWorldSubsystem
{
	GENERATED_BODY()
	
public:
	// 명령어 함수
	void SendCommandToServer(FName CommandName, int32 IntParam = 0, float FloatParam = 0.f, const FString& StringParam = TEXT(""));
	void ProcessServerCommand(APlayerController* InstigatorPC, FName CommandName, int32 IntParam, float FloatParam, const FString& StringParam);


	// --- 1. 봇 제어 ---
	UFUNCTION(BlueprintCallable, Category = "Dev") void AddDummyBot();
	UFUNCTION(BlueprintCallable, Category = "Dev") void RemoveAllDummyBots();
	UFUNCTION(BlueprintCallable, Category = "Dev") void TeleportBotsToMe();

	// --- 2. 매치 흐름 제어 ---
	UFUNCTION(BlueprintCallable, Category = "Dev") void AddGold(int32 Amount);
	UFUNCTION(BlueprintCallable, Category = "Dev") void SetGameSpeed(float SpeedMultiplier);
	UFUNCTION(BlueprintCallable, Category = "Dev") void ForceGameStart();
	UFUNCTION(BlueprintCallable, Category = "Dev") void ForceRoundEnd();
	UFUNCTION(BlueprintCallable, Category = "Dev") void ForceWin();
	UFUNCTION(BlueprintCallable, Category = "Dev") void ForceLose();
	UFUNCTION(BlueprintCallable, Category = "Dev") void SetNextMapByEnum(EMiniGameMapType MapType);

	// --- 3. 캐릭터 제어  ---
	UFUNCTION(BlueprintCallable, Category = "Dev") void ToggleGodMode();
	UFUNCTION(BlueprintCallable, Category = "Dev") void SetSuperSpeed(float SpeedMultiplier);
	UFUNCTION(BlueprintCallable, Category = "Dev") void SetSuperJump(float JumpZVelocity);
	UFUNCTION(BlueprintCallable, Category = "Dev") void ToggleFlyMode();
	UFUNCTION(BlueprintCallable, Category = "Dev") void KillSelf();
	UFUNCTION(BlueprintCallable, Category = "Dev") void RespawnSelf();

	// --- 4. 아이템 & 디버깅 ---
	UFUNCTION(BlueprintCallable, Category = "Dev") void GrantItem(FString ItemID);
	UFUNCTION(BlueprintCallable, Category = "Dev") void TogglePingAndFPS();
	UFUNCTION(BlueprintCallable, Category = "Dev") void ToggleHUD();
	UFUNCTION(BlueprintCallable, Category = "Dev") void ToggleCollisionRender();

private:
	class ACharacter* GetLocalPlayerCharacter() const;

	bool bIsStatVisible = false;
	bool bIsHUDVisible = true;
	bool bIsCollisionVisible = false;
};
