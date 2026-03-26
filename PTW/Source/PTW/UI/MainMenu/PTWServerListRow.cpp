// Fill out your copyright notice in the Description page of Project Settings.


#include "PTWServerListRow.h"
#include "Components/Button.h"
#include "Components/TextBlock.h"
#include "PTW/System/PTWSessionSubsystem.h"
#include "System/PTWGameLiftSubsystem.h"
#include "System/Session/PTWSessionConfig.h"

void UPTWServerListRow::SetupSessionMinimalInfo(const FOnlineSessionSearchResultBP& SearchResult)
{
	SteamSessionInfo = SearchResult.OnlineSessionSearchResult;
	const FOnlineSession& OnlineSession = SteamSessionInfo.Session;
	
	// 위젯 설정
	// 서버 ID 설정
	// FString SessionId = SteamSessionInfo.Session.SessionInfo->GetSessionId().ToString();
	// FString HexRoomCode = FString::Printf(TEXT("%llX"), *SessionId);
	// ServerID->SetText(FText::FromString(HexRoomCode));
	
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
	
	if (!OnlineSession.SessionSettings.bIsDedicated)
	{
		// 리슨 서버 접속
		if (UPTWSessionSubsystem* SessionSubsystem = GameInstance->GetSubsystem<UPTWSessionSubsystem>())
		{
			SessionSubsystem->JoinGameSession(FOnlineSessionSearchResultBP(SteamSessionInfo));
		}
	}
	else
	{
		// 데디케이티드 서버 접속
		FString GameLiftSessionId;
		FString Part1, Part2;
		if (OnlineSession.SessionSettings.Get(FName("GameLiftSessionId_1"), Part1))
		{
			if (!Part1.IsEmpty())
			{
				GameLiftSessionId = Part1;
			}
		}
		if (OnlineSession.SessionSettings.Get(FName("GameLiftSessionId_2"), Part2))
		{
			if (!Part2.IsEmpty())
			{
				GameLiftSessionId += Part2;
			}
		}
		
		UE_LOG(LogTemp, Log, TEXT("GameLiftSessionId: %s"), *GameLiftSessionId);
		if (UPTWGameLiftSubsystem* GameLiftSubsystem = GameInstance->GetSubsystem<UPTWGameLiftSubsystem>())
		{
			const FString UniquePlayerId = GameLiftSubsystem->GetUniquePlayerId();
			GameLiftSubsystem->CreatePlayerSession(UniquePlayerId, GameLiftSessionId);
		}
	}
}
