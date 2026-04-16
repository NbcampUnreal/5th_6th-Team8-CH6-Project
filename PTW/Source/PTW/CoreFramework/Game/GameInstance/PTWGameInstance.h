#pragma once

#include "CoreMinimal.h"
#include "PTWGameInstance.generated.h"

class APTWPlayerState;
class UPTWLoadingWidgetBase;
class USoundMix;
class USoundClass;

USTRUCT(BlueprintType)
struct FReadyPlayerInfo
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString UniqueId = TEXT("");

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString PlayerName = TEXT("");
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	AActor* Owner = nullptr;
};

UENUM(BlueprintType)
enum class ELoadingScreenType : uint8
{
	None,
	Lobby,
	MiniGame
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnPlayerUniqueIdSignature, const FString&, UniqueId);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnPlayerListClearedSignature);

/**
 * PTW GameInstance
 */
UCLASS()
class PTW_API UPTWGameInstance : public UGameInstance
{
	GENERATED_BODY()

public:
	UPTWGameInstance(const FObjectInitializer& ObjectInitializer);
	
	/* 트래블 시작 전 호출하여 로딩 데이터를 셋팅 */
	void PrepareLoadingScreen(ELoadingScreenType InType, FName InMapRowName);
	
	/* MoviePlayer 로딩 화면 시작 */
	UFUNCTION()
	virtual void BeginLoadingScreen(const FString& MapName);
	
	/* MoviePlayer 로딩 화면 수동 시작 */
	void DisplayLoadingScreen();

	/* MoviePlayer 로딩 화면 수동 종료 */
	void StopLoadingScreen();
	
	UFUNCTION(BlueprintCallable, Category = "Network")
	void RegisterPlayerState(APTWPlayerState* PlayerState);
	UFUNCTION(BlueprintCallable, Category = "Network")
	void UnRegisterPlayerState(AActor* DestroyedActor);
	
	void OnWorldInitialized(UWorld* World, const UWorld::InitializationValues IVS);
	UFUNCTION(BlueprintCallable, Category = "Network")
	void RegisterGameState(AGameStateBase* GameState);
	UFUNCTION(BlueprintCallable, Category = "Network")
	void UnRegisterGameState(AActor* Actor, EEndPlayReason::Type EndPlayReason);
	
	UFUNCTION(BlueprintCallable, Category = "Network")
	void HandlePlayerUniqueIdReplicated(APlayerState* PlayerState, const FString& UniqueId);
	UFUNCTION(BlueprintCallable, Category = "Network")
	void HandlePlayerNameReplicated(APlayerState* PlayerState, const FString& PlayerName);
	UFUNCTION(BlueprintCallable, Category = "Network")
	void HandlePlayerOwnerReplicated(APlayerState* PlayerState, AActor* Owner);
	
	UFUNCTION(BlueprintCallable, Category = "Network")
	void CheckAndRegisterPlayer(APlayerState* PlayerState);
	
	UFUNCTION(BlueprintCallable, Category = "Network|Level")
	void AddLevelPlayerId(const FString& UniqueId);
	UFUNCTION(BlueprintCallable, Category = "Network|Level")
	void RemoveLevelPlayerId(const FString& UniqueId);
	UFUNCTION(BlueprintCallable, Category = "Network|Level")
	void ClearLevelPlayerIds();
	
	UFUNCTION(BlueprintCallable, Category = "Network|Session")
	void AddSessionPlayerId(const FString& UniqueId);
	UFUNCTION(BlueprintCallable, Category = "Network|Session")
	void RemoveSessionPlayerId(const FString& UniqueId);
	UFUNCTION(BlueprintCallable, Category = "Network|Session")
	void ClearSessionPlayerIds();
	
public:
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
	UPROPERTY(EditDefaultsOnly, Category = "Audio")
	USoundClass* VoiceSoundClass;
	UPROPERTY(EditDefaultsOnly, Category = "Class")
	TSubclassOf<class ACharacter> KeepCharacterClassLoaded;
	UPROPERTY(EditDefaultsOnly, Category = "Class")
	TSubclassOf<class AActor> KeepResultCharacterClassLoaded;
	
	bool bIsFirstLobby = true;
	int32 CurrentPlayerCount = 0;
	
	UPROPERTY()
	TMap<APlayerState*, FReadyPlayerInfo> ReadyPlayers;
	
	/** 플레이어들의 UniqueId를 보관하는 휘발성 TSet.\n
	 * 레벨이동할때 마다, UniqueId를 Set에 추가/제거를 반복합니다. 
	 */
	UPROPERTY(EditDefaultsOnly, Category = "NetWork")
	TSet<FString> LevelPlayerIds;
	
	/** 플레이어들의 UniqueId를 보관하는 비휘발성 TSet.\n
	 * 현재 세션에 접속/종료 할때마다 Set에 추가/제거를 반복합니다.
	 * 레벨이동 후에도 유지됩니다.
	 */
	UPROPERTY(EditDefaultsOnly, Category = "NetWork")
	TSet<FString> SessionPlayerIds;
	
protected:
	virtual void Init() override;
	virtual void Shutdown() override;
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
	
public:
	UPROPERTY(BlueprintAssignable, Category = "Network|Level")
	FOnPlayerUniqueIdSignature OnLocalPlayerEnteredLevel;
	UPROPERTY(BlueprintAssignable, Category = "Network|Level")
	FOnPlayerUniqueIdSignature OnLocalSessionPlayerConnected;
	
	UPROPERTY(BlueprintAssignable, Category = "Network|Level")
	FOnPlayerUniqueIdSignature OnPlayerEnteredLevel;
	UPROPERTY(BlueprintAssignable, Category = "Network|Level")
	FOnPlayerUniqueIdSignature OnPlayerLeftLevel;
	UPROPERTY(BlueprintAssignable, Category = "Network|Level")
	FOnPlayerListClearedSignature OnLevelPlayersCleared;
	
	UPROPERTY(BlueprintAssignable, Category = "Network|Session")
	FOnPlayerUniqueIdSignature OnSessionPlayerConnected;
	UPROPERTY(BlueprintAssignable, Category = "Network|Session")
	FOnPlayerUniqueIdSignature OnSessionPlayerDisconnected;
	UPROPERTY(BlueprintAssignable, Category = "Network|Session")
	FOnPlayerListClearedSignature OnSessionPlayersCleared;
};
