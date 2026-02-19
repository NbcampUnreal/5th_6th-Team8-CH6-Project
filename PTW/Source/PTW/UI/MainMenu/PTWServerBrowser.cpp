// Fill out your copyright notice in the Description page of Project Settings.


#include "PTWServerBrowser.h"
#include "Components/Border.h"
#include "Components/Button.h"
#include "Components/VerticalBox.h"
#include "Components/EditableText.h"
#include "GameFramework/PlayerState.h"
#include "PTW/UI/MainMenu/PTWServerListRow.h"
#include "PTW/System/PTWSessionSubsystem.h"
#include "System/Session/SessionConfig.h"

#define LOCTEXT_NAMESPACE "ServerBrowser"
void UPTWServerBrowser::NativeConstruct()
{
	Super::NativeConstruct();
	
	RoundLimit = EPTWRoundLimit::Short;
	
	if (!IsValid(ServerListRowClass))
	{
		ServerListRowClass = UPTWServerListRow::StaticClass();
	}
	
	if (IsValid(BackButton))
	{
		BackButton->OnClicked.AddDynamic(this, &ThisClass::OnClickedBackButton);	
	}
	
	if (IsValid(ServerMenuButton))
	{
		ServerMenuButton->OnClicked.AddDynamic(this, &ThisClass::OnClickedServerMenuButton);
	}
	
	if (IsValid(CreateServerButton))
	{
		CreateServerButton->OnClicked.AddDynamic(this, &ThisClass::OnClickedCreateServerButton);
	}
	
	if (IsValid(FindServerButton))
	{
		FindServerButton->OnClicked.AddDynamic(this, &ThisClass::OnClickedFindServerButton);
	}
	
	if (IsValid(ShortRoundButton))
	{
		ShortRoundButton->OnClicked.AddDynamic(this, &ThisClass::OnClickedShortRoundButton);
		OnClickedShortRoundButton();
	}
	
	if (IsValid(LongRoundButton))
	{
		LongRoundButton->OnClicked.AddDynamic(this, &ThisClass::OnClickedLongRoundButton);
	}
	
	UGameInstance* GameInstance = GetGameInstance();
	if (IsValid(GameInstance))
	{
		UPTWSessionSubsystem* SessionSubsystem = GameInstance->GetSubsystem<UPTWSessionSubsystem>();
		if (IsValid(SessionSubsystem))
		{
			SessionSubsystem->OnSessionSearchComplete.AddDynamic(this, &ThisClass::OnFindSessionsComplete);
		}
	}
	
	if (IsValid(ServerNameEditableText))
	{
		if (APlayerState* PS = GetOwningPlayerState())
		{
			FText NewServerName = FText::Format(LOCTEXT("sServer", "{0}'s Server"), FText::FromString(PS->GetPlayerName()));
			ServerNameEditableText->SetText(NewServerName);
		}
	}
	
	if (IsValid(ServerMaxPlayerEditableText))
	{
		ServerMaxPlayerEditableText->SetText(FText::FromString(TEXT("16")));
	}
}

void UPTWServerBrowser::NativeDestruct()
{
	Super::NativeDestruct();
	
	if (IsValid(BackButton))
	{
		BackButton->OnClicked.RemoveDynamic(this, &ThisClass::OnClickedBackButton);	
	}
}

void UPTWServerBrowser::OnClickedBackButton()
{
	if (OnServerBackAction.IsBound())
	{
		OnServerBackAction.Broadcast();
	}
}

void UPTWServerBrowser::OnClickedServerMenuButton()
{
	if (!IsValid(ServerMenuBorder)) return;
	
	if (ServerMenuBorder->GetVisibility() == ESlateVisibility::Collapsed)
	{
		// UI 숨김 -> 보임
		ServerMenuBorder->SetVisibility(ESlateVisibility::Visible);
	}
	else
	{
		// UI 보임 -> 숨김
		ServerMenuBorder->SetVisibility(ESlateVisibility::Collapsed);
	}
}

void UPTWServerBrowser::OnClickedCreateServerButton()
{
	FSessionConfig SessionConfig;
	if (IsValid(ServerNameEditableText))
	{
		SessionConfig.ServerName = ServerNameEditableText->GetText().ToString();
		
	}
	
	if (IsValid(ServerMaxPlayerEditableText))
	{
		SessionConfig.MaxPlayers = FCString::Atoi(*ServerMaxPlayerEditableText->GetText().ToString());
	}
	
	if (IsValid(ServerMaxPlayerEditableText))
	{
		SessionConfig.MaxRounds = GetMaxRoundsByLimit(RoundLimit);
	}
	
	UGameInstance* GameInstance = GetGameInstance();
	if (!IsValid(GameInstance)) return;
	
	UPTWSessionSubsystem* SessionSubsystem = GameInstance->GetSubsystem<UPTWSessionSubsystem>();
	if (!IsValid(SessionSubsystem)) return;
	
	SessionSubsystem->CreateGameSession(SessionConfig);
	// SessionSubsystem->LaunchDedicatedServer(ServerSettings, 16, false);
}

void UPTWServerBrowser::OnClickedFindServerButton()
{
	ServerListVerticalBox->ClearChildren();
	
	if (!IsValid(ServerNameEditableText)) return;
	
	UGameInstance* GameInstance = GetGameInstance();
	if (!IsValid(GameInstance)) return;
	
	UPTWSessionSubsystem* SessionSubsystem = GameInstance->GetSubsystem<UPTWSessionSubsystem>();
	if (!IsValid(SessionSubsystem)) return;
	
	SessionSubsystem->FindGameSession();
}

void UPTWServerBrowser::OnClickedShortRoundButton()
{
	RoundLimit = EPTWRoundLimit::Short;
	
	ShortRoundButton->SetBackgroundColor(FLinearColor::Green);
	LongRoundButton->SetBackgroundColor(FLinearColor::White);
}

void UPTWServerBrowser::OnClickedLongRoundButton()
{
	RoundLimit = EPTWRoundLimit::Long;
	
	LongRoundButton->SetBackgroundColor(FLinearColor::Green);
	ShortRoundButton->SetBackgroundColor(FLinearColor::White);
}

void UPTWServerBrowser::OnFindSessionsComplete(const TArray<FOnlineSessionSearchResultBP>& SearchResults)
{
	if (SearchResults.IsEmpty()) return;
	
	for (const FOnlineSessionSearchResultBP& SearchResult : SearchResults)
	{
		UPTWServerListRow* ServerListRow = CreateWidget<UPTWServerListRow>(this, ServerListRowClass);
		ServerListRow->Setup(SearchResult);
		ServerListVerticalBox->AddChildToVerticalBox(ServerListRow);
	}
}
#undef LOCTEXT_NAMESPACE
