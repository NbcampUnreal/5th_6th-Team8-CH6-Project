// Fill out your copyright notice in the Description page of Project Settings.


#include "PTWServerListRow.h"
#include "Components/Button.h"
#include "Components/TextBlock.h"
#include "PTW/System/PTWSessionSubsystem.h"
#include "System/PTWGameLiftSubsystem.h"
#include "System/Session/PTWSessionConfig.h"

void UPTWServerListRow::SetupSteamInfo(const FOnlineSessionSearchResultBP& SearchResult)
{
	SteamSessionInfo = SearchResult.OnlineSessionSearchResult;
	const FSessionSettings& SessionSettings = SteamSessionInfo.Session.SessionSettings.Settings;
	
	FString SessionId = SteamSessionInfo.Session.SessionInfo->GetSessionId().ToString();
	FString HexRoomCode = FString::Printf(TEXT("%llX"), *SessionId);
	
	SessionConfig.ServerID = TEXT("STEAM-") + HexRoomCode;
	ServerID->SetText(FText::FromString(SessionConfig.ServerID));
	
	FString PureSessionName;
	if (SteamSessionInfo.Session.SessionSettings.Get(FName(PTWSessionKey::ServerName), PureSessionName))
	{
		SessionConfig.ServerName = PureSessionName;
		ServerName->SetText(FText::FromString(SessionConfig.ServerName));
	}
	SessionConfig.bUseGameLift = false;
}

void UPTWServerListRow::SetupGameLiftInfo(const FPTWGameLiftGameSession& SearchResult)
{
	GameLiftSessionInfo = SearchResult;
	
	FString SessionId = GameLiftSessionInfo.GameSessionId;
	FString RoomCode; // FString::Printf(TEXT("%llX"), SessionId);
	int32 LastSlashIndex;
	if (SessionId.FindLastChar('/', LastSlashIndex))
	{
		FString UUIDString = SessionId.RightChop(LastSlashIndex + 1);
		FString LeftPart, RightPart;
		if (UUIDString.Split(TEXT("-"), &LeftPart, &RightPart))
		{
			RoomCode = LeftPart;
		}
		else
		{
			RoomCode = UUIDString.Left(8);
		}
	}
	else
	{
		RoomCode = SessionId.Left(8);
	}
	SessionConfig.ServerID = TEXT("AWS-") + RoomCode;
	ServerID->SetText(FText::FromString(SessionConfig.ServerID));
	
	SessionConfig.ServerName = GameLiftSessionInfo.Name;
	ServerName->SetText(FText::FromString(SessionConfig.ServerName));
	
	SessionConfig.bUseGameLift = true;
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
	
	if (!SessionConfig.bUseGameLift)
	{
		if (UPTWSessionSubsystem* SessionSubsystem = GameInstance->GetSubsystem<UPTWSessionSubsystem>())
		{
			SessionSubsystem->JoinGameSession(FOnlineSessionSearchResultBP(SteamSessionInfo));
		}
	}
	else
	{
		if (UPTWGameLiftSubsystem* GameLiftSubsystem = GameInstance->GetSubsystem<UPTWGameLiftSubsystem>())
		{
			GameLiftSubsystem->DescribeGameSession(GameLiftSessionInfo.GameSessionId);
		}
	}
}
