// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "PTWSpawnComponent.generated.h"


struct FPTWMiniGameRule;
class APTWPlayerController;

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class PTW_API UPTWSpawnComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	UPTWSpawnComponent();


	void InitSpawnComponent(const FPTWMiniGameRule* InMiniGameRule);
	//* 리스폰 타이머 시작 함수 */
	void RespawnPlayer(APTWPlayerController* SpawnPlayerController);
	//* RestartPlayer함수 실행  */
	void HandleRespawn(APTWPlayerController* PlayerController);

private:
	const FPTWMiniGameRule* MiniGameRule;
};
