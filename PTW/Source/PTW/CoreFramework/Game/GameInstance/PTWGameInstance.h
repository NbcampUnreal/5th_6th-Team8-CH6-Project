// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "PTWGameInstance.generated.h"

class UPTWLoadingWidgetBase;
class USoundMix;
class USoundClass;

UENUM(BlueprintType)
enum class ELoadingScreenType : uint8
{
	None,
	Lobby,
	MiniGame
};

/**
 * 
 */
UCLASS()
class PTW_API UPTWGameInstance : public UGameInstance
{
	GENERATED_BODY()


public:
	UPTWGameInstance(const FObjectInitializer& ObjectInitializer);

	bool bIsFirstLobby = true;
	int32 CurrentPlayerCount =0;
	
	virtual void Init() override;

	/* 트래블 시작 전 호출하여 로딩 데이터를 셋팅 */
	void PrepareLoadingScreen(ELoadingScreenType InType, FName InMapRowName);

	/* MoviePlayer 로딩 화면 시작 */
	UFUNCTION()
	virtual void BeginLoadingScreen(const FString& MapName);

	/* MoviePlayer 로딩 화면 수동 시작 */
	void DisplayLoadingScreen();

	/* MoviePlayer 로딩 화면 수동 종료 */
	void StopLoadingScreen();

	/* 게임 시작 시 사운드 설정 저장값 자동 적용 */
	UPROPERTY(EditDefaultsOnly, Category = "Audio")
	USoundMix* MasterSoundMix;
	UPROPERTY(EditDefaultsOnly, Category = "Audio")
	USoundClass* MasterSoundClass;
	UPROPERTY(EditDefaultsOnly, Category = "Audio")
	USoundClass* BGMSoundClass;
	UPROPERTY(EditDefaultsOnly, Category = "Audio")
	USoundClass* SFXSoundClass;
	UPROPERTY(EditDefaultsOnly, Category = "Audio")
	USoundClass* UISoundClass;

protected:
	UPROPERTY()
	ELoadingScreenType NextLoadingType; // 다음이 미니게임인지 로비인지 저장

	UPROPERTY()
	FName TargetMapRowName; // 맵 이름 저장

	/* 로딩 위젯 클래스들(에디터에서 할당) */
	UPROPERTY(EditDefaultsOnly, Category = "UI")
	TSubclassOf<UPTWLoadingWidgetBase> LobbyLoadingWidgetClass;
	UPROPERTY(EditDefaultsOnly, Category = "UI")
	TSubclassOf<UPTWLoadingWidgetBase> MiniGameLoadingWidgetClass;

	/* 맵 데이터 테이블 */
	UPROPERTY(EditDefaultsOnly, Category = "Loading")
	UDataTable* MiniGameMapTable;

	/* 로비일 때 사용할 기본 배경 이미지 */
	UPROPERTY(EditDefaultsOnly, Category = "Loading")
	TSoftObjectPtr<UTexture2D> LobbyDefaultImage;
};
