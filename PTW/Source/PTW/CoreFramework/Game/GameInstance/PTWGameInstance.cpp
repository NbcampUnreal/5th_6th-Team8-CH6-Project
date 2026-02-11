// Fill out your copyright notice in the Description page of Project Settings.


#include "CoreFramework/Game/GameInstance/PTWGameInstance.h"
#include "MoviePlayer.h"
#include "Blueprint/UserWidget.h"
#include "MiniGame/PTWMiniGameMapRow.h"
#include "UI/LoadingScreen/PTWLoadingMiniGame.h"
#include "UI/LoadingScreen/PTWLoadingWidgetBase.h"

void UPTWGameInstance::Init()
{
	Super::Init();

	FCoreUObjectDelegates::PreLoadMap.AddUObject(this, &UPTWGameInstance::BeginLoadingScreen);
}

void UPTWGameInstance::PrepareLoadingScreen(ELoadingScreenType InType, FName InMapRowName)
{
	NextLoadingType = InType;
	TargetMapRowName = InMapRowName;
}

void UPTWGameInstance::BeginLoadingScreen(const FString& MapName)
{
	if (IsRunningDedicatedServer()) return;

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
				MiniGameWidget->SetupMiniGameInfo(MapData->DisplayName, FText::FromString(TEXT("미니게임 설명...")));
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
	LoadingScreen.WidgetLoadingScreen = LoadingWidget->TakeWidget();
	GetMoviePlayer()->SetupLoadingScreen(LoadingScreen);
}

void UPTWGameInstance::StopLoadingScreen()
{
	if (GetMoviePlayer()->IsMovieCurrentlyPlaying())
	{
		GetMoviePlayer()->StopMovie();
	}
}
