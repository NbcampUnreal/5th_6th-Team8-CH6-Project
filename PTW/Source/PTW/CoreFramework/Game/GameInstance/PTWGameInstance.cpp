#include "CoreFramework/Game/GameInstance/PTWGameInstance.h"
#include "MoviePlayer.h"
#include "Blueprint/UserWidget.h"
#include "MiniGame/PTWMiniGameMapRow.h"
#include "UI/LoadingScreen/PTWLoadingMiniGame.h"
#include "UI/LoadingScreen/PTWLoadingWidgetBase.h"
#include "CoreFramework/PTWGameUserSettings.h"
#include "CoreFramework/PTWPlayerState.h"
#include "GameFramework/GameStateBase.h"

UPTWGameInstance::UPTWGameInstance(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	NextLoadingType = ELoadingScreenType::None;
}
void UPTWGameInstance::Init()
{
	Super::Init();

	FCoreUObjectDelegates::PreLoadMap.AddUObject(this, &UPTWGameInstance::BeginLoadingScreen);

	/* 사운드 저장값 자동 적용 */
	if (!GEngine) return;

	UPTWGameUserSettings* Settings = Cast<UPTWGameUserSettings>(GEngine->GetGameUserSettings());
	if (!Settings) return;

	// 저장된 설정 로드
	Settings->LoadSettings(false);

	// 오디오 적용
	Settings->ApplyAudioSettings(
		GetWorld(),
		MasterSoundMix,
		MasterSoundClass,
		BGMSoundClass,
		SFXSoundClass,
		UISoundClass,
		VoiceSoundClass
	);
	FWorldDelegates::OnPostWorldInitialization.AddUObject(this, &ThisClass::OnWorldInitialized);
}

void UPTWGameInstance::Shutdown()
{
	FWorldDelegates::OnPostWorldInitialization.RemoveAll(this);
	Super::Shutdown();
}

void UPTWGameInstance::PrepareLoadingScreen(ELoadingScreenType InType, FName InMapRowName)
{
	NextLoadingType = InType;
	TargetMapRowName = InMapRowName;
}

void UPTWGameInstance::BeginLoadingScreen(const FString& MapName)
{
	DisplayLoadingScreen();
}

void UPTWGameInstance::DisplayLoadingScreen()
{
	if (IsRunningDedicatedServer()) return;

	if (NextLoadingType == ELoadingScreenType::None)
	{
		return;
	}

	// 위젯 클래스 결정
	TSubclassOf<UPTWLoadingWidgetBase> TargetClass = (NextLoadingType == ELoadingScreenType::Lobby)
		? LobbyLoadingWidgetClass
		: MiniGameLoadingWidgetClass;

	if (!TargetClass)
	{
		return;
	}
	// 위젯 생성 (타입을 구체적으로 지정)
	UPTWLoadingWidgetBase* LoadingWidget = CreateWidget<UPTWLoadingWidgetBase>(this, TargetClass);
	if (!LoadingWidget) return;

	// 데이터 세팅
	if (NextLoadingType == ELoadingScreenType::MiniGame && MiniGameMapTable)
	{
		static const FString ContextString(TEXT("LoadingContext"));
		FPTWMiniGameMapRow* MapData = MiniGameMapTable->FindRow<FPTWMiniGameMapRow>(TargetMapRowName, ContextString);

		if (MapData)
		{
			// 공통 이미지 세팅 (UPTWLoadingWidgetBase의 함수)
			LoadingWidget->InitBaseUI(MapData->Thumbnail);

			// 미니게임 전용 텍스트 세팅 (자식으로 캐스팅하여 호출)
			if (UPTWLoadingMiniGame* MiniGameWidget = Cast<UPTWLoadingMiniGame>(LoadingWidget))
			{
				MiniGameWidget->SetupMiniGameInfo(MapData->DisplayName, MapData->MapDescription);
			}
		}
	}
	else // 로비일 때
	{
		// 클래스에 추가한 LobbyDefaultImage 사용
		LoadingWidget->InitBaseUI(LobbyDefaultImage);
	}

	// MoviePlayer 등록
	FLoadingScreenAttributes LoadingScreen;
	LoadingScreen.bAutoCompleteWhenLoadingCompletes = false;
	LoadingScreen.MinimumLoadingScreenDisplayTime = 3.0f;
	LoadingScreen.WidgetLoadingScreen = LoadingWidget->TakeWidget();
	GetMoviePlayer()->SetupLoadingScreen(LoadingScreen);

	GetMoviePlayer()->PlayMovie();
	NextLoadingType = ELoadingScreenType::None;
}

void UPTWGameInstance::StopLoadingScreen()
{
	if (GetMoviePlayer()->IsMovieCurrentlyPlaying())
	{
		GetMoviePlayer()->StopMovie();
	}
}

void UPTWGameInstance::RegisterPlayerState(APTWPlayerState* PlayerState)
{
	if (!PlayerState) return;
	
	PlayerState->OnPlayerUniqueIdReplicated.AddDynamic(this, &ThisClass::HandlePlayerUniqueIdReplicated);
	PlayerState->OnPlayerNameReplicated.AddDynamic(this, &ThisClass::HandlePlayerNameReplicated);
	PlayerState->OnPlayerOwnerReplicated.AddDynamic(this, &ThisClass::HandlePlayerOwnerReplicated);
	
	PlayerState->OnDestroyed.RemoveDynamic(this, &ThisClass::UnRegisterPlayerState);
}

void UPTWGameInstance::UnRegisterPlayerState(AActor* DestroyedActor)
{
	APTWPlayerState* PlayerState = Cast<APTWPlayerState>(DestroyedActor);
	if (!PlayerState) return;
	
	PlayerState->OnPlayerUniqueIdReplicated.RemoveDynamic(this, &ThisClass::HandlePlayerUniqueIdReplicated);
	PlayerState->OnPlayerNameReplicated.RemoveDynamic(this, &ThisClass::HandlePlayerNameReplicated);
	PlayerState->OnPlayerOwnerReplicated.RemoveDynamic(this, &ThisClass::HandlePlayerOwnerReplicated);
}

void UPTWGameInstance::OnWorldInitialized(UWorld* World, const UWorld::InitializationValues IVS)
{
	if (World && World->IsGameWorld())
	{
		World->GameStateSetEvent.AddUObject(this, &ThisClass::RegisterGameState);
	}
}

void UPTWGameInstance::RegisterGameState(AGameStateBase* GameState)
{
	GameState->OnEndPlay.AddUniqueDynamic(this, &ThisClass::UnRegisterGameState);
}

void UPTWGameInstance::UnRegisterGameState(AActor* Actor, EEndPlayReason::Type EndPlayReason)
{
	if (Actor)
	{
		Actor->OnEndPlay.RemoveDynamic(this, &ThisClass::UnRegisterGameState);
	}
	
	ClearLevelPlayerIds();
}

void UPTWGameInstance::HandlePlayerUniqueIdReplicated(APlayerState* PlayerState, const FString& UniqueId)
{
	if (!ReadyPlayers.Contains(PlayerState))
	{
		ReadyPlayers.Add(PlayerState, FReadyPlayerInfo());
	}
	
	ReadyPlayers[PlayerState].UniqueId = UniqueId;
	CheckAndRegisterPlayer(PlayerState);
}

void UPTWGameInstance::HandlePlayerNameReplicated(APlayerState* PlayerState, const FString& PlayerName)
{
	if (PlayerName == TEXT("Player")) return;
	if (!ReadyPlayers.Contains(PlayerState))
	{
		ReadyPlayers.Add(PlayerState, FReadyPlayerInfo());
	}
	
	ReadyPlayers[PlayerState].PlayerName = PlayerName;
	CheckAndRegisterPlayer(PlayerState);
}

void UPTWGameInstance::HandlePlayerOwnerReplicated(APlayerState* PlayerState, AActor* Owner)
{
	if (!IsValid(Owner)) return;
	if (!ReadyPlayers.Contains(PlayerState))
	{
		ReadyPlayers.Add(PlayerState, FReadyPlayerInfo());
	}
	
	ReadyPlayers[PlayerState].Owner = Owner;
	CheckAndRegisterPlayer(PlayerState);
}

void UPTWGameInstance::CheckAndRegisterPlayer(APlayerState* PlayerState)
{
	const FReadyPlayerInfo& ReadyPlayerInfo= ReadyPlayers[PlayerState];
	
	if (!ReadyPlayerInfo.UniqueId.IsEmpty() && !ReadyPlayerInfo.PlayerName.IsEmpty())
	{
		const FString& UniqueId = ReadyPlayers[PlayerState].UniqueId;
		AddLevelPlayerId(UniqueId);
		AddSessionPlayerId(UniqueId);
		
		if (IsValid(ReadyPlayerInfo.Owner))
		{
			// My UniqueId
			OnLocalPlayerEnteredLevel.Broadcast(ReadyPlayerInfo.UniqueId);
		}
	}
}

void UPTWGameInstance::AddLevelPlayerId(const FString& UniqueId)
{
	if (LevelPlayerIds.Contains(UniqueId)) return;
	LevelPlayerIds.Add(UniqueId);
	OnPlayerEnteredLevel.Broadcast(UniqueId);
}

void UPTWGameInstance::RemoveLevelPlayerId(const FString& UniqueId)
{
	LevelPlayerIds.Remove(UniqueId);
	OnPlayerLeftLevel.Broadcast(UniqueId);
}

void UPTWGameInstance::ClearLevelPlayerIds()
{
	LevelPlayerIds.Empty();
	OnLevelPlayersCleared.Broadcast();
}

void UPTWGameInstance::AddSessionPlayerId(const FString& UniqueId)
{
	if (SessionPlayerIds.Contains(UniqueId)) return;
	SessionPlayerIds.Add(UniqueId);
	OnSessionPlayerConnected.Broadcast(UniqueId);
}

void UPTWGameInstance::RemoveSessionPlayerId(const FString& UniqueId)
{
	SessionPlayerIds.Remove(UniqueId);
	OnSessionPlayerDisconnected.Broadcast(UniqueId);
}

void UPTWGameInstance::ClearSessionPlayerIds()
{
	SessionPlayerIds.Empty();
	OnSessionPlayersCleared.Broadcast();
}
