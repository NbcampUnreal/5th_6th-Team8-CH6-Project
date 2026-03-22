// Fill out your copyright notice in the Description page of Project Settings.


#include "PTWServerBrowser.h"
#include "Components/Border.h"
#include "Components/Button.h"
#include "Components/CheckBox.h"
#include "Components/VerticalBox.h"
#include "Components/EditableText.h"
#include "GameFramework/PlayerState.h"
#include "Kismet/GameplayStatics.h"
#include "PTW/UI/MainMenu/PTWServerListRow.h"
#include "PTW/System/PTWSessionSubsystem.h"
#include "System/PTWGameLiftSubsystem.h"
#include "System/Session/PTWSessionConfig.h"

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
	
	if (IsValid(QuickMatchButton))
	{
		QuickMatchButton->OnClicked.AddDynamic(this, &ThisClass::OnClickedQuickMatchButton);
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
	
	if (IsValid(TestButton))
	{
		TestButton->OnClicked.AddDynamic(this, &ThisClass::OnClickedTestButton);
	}
	
	if (IsValid(DevJoinButton))
	{
		#if WITH_EDITOR || 1
		DevJoinButton->SetVisibility(ESlateVisibility::Visible);
		DevJoinButton->OnClicked.AddDynamic(this, &ThisClass::DevJoinAction);
		#endif
	}
	
	UGameInstance* GameInstance = GetGameInstance();
	if (IsValid(GameInstance))
	{
		if (UPTWSessionSubsystem* SessionSubsystem = GameInstance->GetSubsystem<UPTWSessionSubsystem>())
		{
			SessionSubsystem->OnSessionSearchComplete.AddDynamic(this, &ThisClass::OnFindSteamSessionsComplete);
		}
		
		if (UPTWGameLiftSubsystem* GameLiftSubsystem = GameInstance->GetSubsystem<UPTWGameLiftSubsystem>())
		{
			GameLiftSubsystem->OnSessionSearchComplete.AddDynamic(this, &ThisClass::OnFindGameLiftSessionsComplete);
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
	UGameInstance* GameInstance = GetGameInstance();
	if (!IsValid(GameInstance)) return;
	
	FPTWSessionConfig SessionConfig;
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
	SessionConfig.bIsDedicatedServer = DedicatedCheckBox->IsChecked();
	
	if (SessionConfig.bIsDedicatedServer)
	{
		SessionConfig.bUseGameLift = true;
		// Dedicated Server는 AWS GameLift에서 원격으로 Fleet Instance에 빈 프로세스를 선택하고 GameSession을 생성.
		if (UPTWGameLiftSubsystem* GameLiftSubsystem = GameInstance->GetSubsystem<UPTWGameLiftSubsystem>())
		{
			GameLiftSubsystem->CreateGameSession(SessionConfig);
		}
	}
	else
	{
		SessionConfig.bUseGameLift = false;
		// Listen Server는 현재 Desktop 에서 실행.
		if (UPTWSessionSubsystem* SessionSubsystem = GameInstance->GetSubsystem<UPTWSessionSubsystem>())
		{
			SessionSubsystem->CreateGameSession(SessionConfig, true);
		}
	}
}

void UPTWServerBrowser::OnClickedFindServerButton()
{
	ServerListVerticalBox->ClearChildren();
	
	if (!IsValid(ServerNameEditableText)) return;
	
	UGameInstance* GameInstance = GetGameInstance();
	if (!IsValid(GameInstance)) return;
	
	if (UPTWSessionSubsystem* SessionSubsystem = GameInstance->GetSubsystem<UPTWSessionSubsystem>())
	{
		SessionSubsystem->FindGameSession();
	}	
	if (UPTWGameLiftSubsystem* GameLiftSubsystem = GameInstance->GetSubsystem<UPTWGameLiftSubsystem>())
	{
		GameLiftSubsystem->SearchGameSessions();
	}
}

void UPTWServerBrowser::OnClickedQuickMatchButton()
{
	UGameInstance* GameInstance = GetGameInstance();
	if (!IsValid(GameInstance)) return;
	
	UPTWSessionSubsystem* SessionSubsystem = GameInstance->GetSubsystem<UPTWSessionSubsystem>();
	if (!IsValid(SessionSubsystem)) return;
	
	SessionSubsystem->QuickMatchGameSession();
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

void UPTWServerBrowser::OnFindSteamSessionsComplete(const TArray<FOnlineSessionSearchResultBP>& SearchResults)
{
	if (SearchResults.IsEmpty()) return;
	
	for (const FOnlineSessionSearchResultBP& SearchResult : SearchResults)
	{
		UPTWServerListRow* ServerListRow = CreateWidget<UPTWServerListRow>(this, ServerListRowClass);
		ServerListRow->SetupSteamInfo(SearchResult);
		ServerListVerticalBox->AddChildToVerticalBox(ServerListRow);
	}
}

void UPTWServerBrowser::OnFindGameLiftSessionsComplete(const TArray<FPTWGameLiftGameSession>& SearchResults)
{
	if (SearchResults.IsEmpty()) return;
	
	for (const FPTWGameLiftGameSession& SearchResult : SearchResults)
	{
		UPTWServerListRow* ServerListRow = CreateWidget<UPTWServerListRow>(this, ServerListRowClass);
		ServerListRow->SetupGameLiftInfo(SearchResult);
		ServerListVerticalBox->AddChildToVerticalBox(ServerListRow);
	}
}

void UPTWServerBrowser::OnClickedTestButton()
{
	/*
	HTTPRequestManager->OnListFleetsResponseReceived.AddUniqueDynamic(this, &ThisClass::OnListFleetsResponseReceived);
	HTTPRequestManager->RequestListFleets();
	TestButton->SetIsEnabled(false);
	*/
	
	// HTTPRequestManager->JoinGameSession();
	TestButton->SetIsEnabled(false);
}

void UPTWServerBrowser::DevJoinAction()
{
	UPTWGameLiftSubsystem::SetNetDriverToIP();
	UGameplayStatics::OpenLevel(GetWorld(), TEXT("59.30.255.170:7777"));
}
#undef LOCTEXT_NAMESPACE
