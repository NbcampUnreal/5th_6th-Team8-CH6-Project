#include "CoreFramework/Game/GameInstance/PTWGameInstance.h"
#include "MoviePlayer.h"
#include "Blueprint/UserWidget.h"
#include "MiniGame/PTWMiniGameMapRow.h"
#include "UI/LoadingScreen/PTWLoadingMiniGame.h"
#include "UI/LoadingScreen/PTWLoadingWidgetBase.h"
#include "CoreFramework/PTWGameUserSettings.h"

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

void UPTWGameInstance::AddPlayerUniqueId(const FString& UniqueId)
{
	PlayerUniqueIds.Add(UniqueId);
	OnPlayerConnected.Broadcast(UniqueId);
}

void UPTWGameInstance::RemovePlayerUniqueId(const FString& UniqueId)
{
	PlayerUniqueIds.Remove(UniqueId);
	OnPlayerDisconnected.Broadcast(UniqueId);
}

void UPTWGameInstance::RemoveAllPlayerUniqueId()
{
	PlayerUniqueIds.Empty();
	OnAllPlayersDisconnected.Broadcast(FString());
}
