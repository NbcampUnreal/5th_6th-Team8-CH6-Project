// Fill out your copyright notice in the Description page of Project Settings.


#include "PTWServerListRow.h"
#include "Components/Button.h"
#include "Components/TextBlock.h"
#include "PTW/System/PTWSessionSubsystem.h"
#include "System/Session/PTWSessionConfig.h"

void UPTWServerListRow::Setup(const FOnlineSessionSearchResultBP& SearchResult)
{
	SessionData = SearchResult.OnlineSessionSearchResult;
	const FSessionSettings& SessionSettings = SessionData.Session.SessionSettings.Settings;
	
	if (SessionSettings.Find(PTWSessionKey::ServerName))
	{
		FString ServerNameStr = SessionSettings.Find(PTWSessionKey::ServerName)->ToString();
		ServerName->SetText(FText::FromString(ServerNameStr));
	}
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
	
	UPTWSessionSubsystem* SessionSubsystem = GameInstance->GetSubsystem<UPTWSessionSubsystem>();
	if (!IsValid(SessionSubsystem)) return;
	
	SessionSubsystem->JoinGameSession(FOnlineSessionSearchResultBP(SessionData));
}
