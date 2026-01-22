// Fill out your copyright notice in the Description page of Project Settings.


#include "PTWLobbyListRow.h"
#include "Components/Button.h"
#include "Components/TextBlock.h"
#include "PTW/System/PTWSessionSubsystem.h"

void UPTWLobbyListRow::Setup(const FBlueprintSessionResult& SessionResult)
{
	SessionData = SessionResult;
	
	const FOnlineSessionSearchResult& NativeResult = SessionResult.OnlineResult;
	const FSessionSettings& SessionSettings = NativeResult.Session.SessionSettings.Settings;
	
	FString LobbyNameStr = SessionSettings.Find("LobbyName")->ToString();
	// LobbyID->SetText();
	LobbyName->SetText(FText::FromString(LobbyNameStr));
}

void UPTWLobbyListRow::NativeConstruct()
{
	Super::NativeConstruct();
	
	if (IsValid(JoinButton))
	{
		JoinButton->OnClicked.AddDynamic(this, &ThisClass::OnClickedJoinButton);
	}
}

void UPTWLobbyListRow::NativeDestruct()
{
	if (!IsValid(JoinButton))
	{
		JoinButton->OnClicked.RemoveDynamic(this, &ThisClass::OnClickedJoinButton);
	}
	
	Super::NativeDestruct();
}

void UPTWLobbyListRow::OnClickedJoinButton()
{
	UGameInstance* GameInstance = GetGameInstance();
	if (!IsValid(GameInstance))
	{
		return;
	}
	
	UPTWSessionSubsystem* SessionSubsystem = GameInstance->GetSubsystem<UPTWSessionSubsystem>();
	if (!IsValid(SessionSubsystem))
	{
		return;
	}
	
	SessionSubsystem->JoinLobbySession(SessionData);
}
