// Fill out your copyright notice in the Description page of Project Settings.


#include "PTWServerListRow.h"
#include "Components/Button.h"
#include "Components/TextBlock.h"
#include "PTW/System/PTWSteamSessionSubsystem.h"
#include "System/PTWGameLiftClientSubsystem.h"
#include "System/Session/PTWSessionConfig.h"

void UPTWServerListRow::SetupSessionMinimalInfo(const FOnlineSessionSearchResultBP& SearchResult)
{
	SteamSessionInfo = SearchResult.OnlineSessionSearchResult;
	const FOnlineSession& OnlineSession = SteamSessionInfo.Session;
	
	// 위젯 설정
	// 서버 이름 설정
	FString ServerNameStr;
	if (OnlineSession.SessionSettings.Get(FName(PTWSessionKey::ServerName), ServerNameStr))
	{
		ServerName->SetText(FText::FromString(ServerNameStr));
	}
	
	// 라운드 유형 설정
	int32 RoundLimitTypeInt32;
	if (OnlineSession.SessionSettings.Get(FName(PTWSessionKey::MaxRounds), RoundLimitTypeInt32))
	{
		if (RoundLimitTypeInt32 == GetMaxRoundsByLimit(EPTWRoundLimit::Short))
		{
			RoundLimitType->SetText(FText::FromString(TEXT("Short")));
		}
		else
		{
			RoundLimitType->SetText(FText::FromString(TEXT("Long")));
		}
	}
	
	// 플레이어 수 설정
	int32 OpenSlots = OnlineSession.NumOpenPublicConnections;
	int32 MaxPlayers = OnlineSession.SessionSettings.NumPublicConnections;
	
	FString ServerPlayersStr = FString::Printf(TEXT("%d / %d"), 
		MaxPlayers - OpenSlots, MaxPlayers);
	ServerPlayers->SetText(FText::FromString(ServerPlayersStr));
	
	// 핑 설정
	FString ServerPingStr = FString::Printf((TEXT("%dms")), SteamSessionInfo.PingInMs);
	ServerPing->SetText(FText::FromString(ServerPingStr));
}

void UPTWServerListRow::NativeConstruct()
{
	Super::NativeConstruct();
	
	if (IsValid(JoinButton))
	{
		JoinButton->OnClicked.AddDynamic(this, &ThisClass::OnClickedJoinButton);
	}
}

void UPTWServerListRow::NativeDestruct()
{
	if (!IsValid(JoinButton))
	{
		JoinButton->OnClicked.RemoveDynamic(this, &ThisClass::OnClickedJoinButton);
	}
	
	Super::NativeDestruct();
}

void UPTWServerListRow::OnClickedJoinButton()
{
	UGameInstance* GameInstance = GetGameInstance();
	if (!IsValid(GameInstance)) return;
	
	const FOnlineSession& OnlineSession = SteamSessionInfo.Session;
	bool bIsNoGameLift = false;
	const FOnlineSessionSearchResultBP& SteamSessionInfoBP = FOnlineSessionSearchResultBP(SteamSessionInfo);
	
	OnlineSession.SessionSettings.Get(PTWSessionKey::NoGameLift, bIsNoGameLift);
	
	if (!OnlineSession.SessionSettings.bIsDedicated || bIsNoGameLift)
	{
		// 리슨 서버 접속
		if (UPTWSteamSessionSubsystem* SteamSessionSubsystem = UPTWSteamSessionSubsystem::Get(this))
		{
			SteamSessionSubsystem->JoinGameSession(SteamSessionInfoBP);
		}
	}
	else
	{
		// 데디케이티드 서버 접속
		// FString GameLiftSessionId;
		// OnlineSession.SessionSettings.Get(PTWSessionKey::GameLiftSessionId, GameLiftSessionId);
		//
		// if (GameLiftSessionId.IsEmpty()) return;
		//
		// if (UPTWGameLiftClientSubsystem* GameLiftClientSubsystem = UPTWGameLiftClientSubsystem::Get(this))
		// {
		// 	const FString UniquePlayerId = GameLiftClientSubsystem->GetUniquePlayerId();
		// 	GameLiftClientSubsystem->CreatePlayerSession(UniquePlayerId, GameLiftSessionId);
		// }
	}
}
