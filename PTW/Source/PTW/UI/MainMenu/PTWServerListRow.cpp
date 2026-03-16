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
	
	if (SessionSettings.Find(PTWSessionKey::ServerName))
	{
		SessionConfig.ServerName = SessionSettings.Find(PTWSessionKey::ServerName)->ToString();
		ServerName->SetText(FText::FromString(SessionConfig.ServerName));
	}
	SessionConfig.bUseGameLift = false;
}

void UPTWServerListRow::SetupGameLiftInfo(const FPTWGameLiftGameSession& SearchResult)
{
	GameLiftSessionInfo = SearchResult;
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
