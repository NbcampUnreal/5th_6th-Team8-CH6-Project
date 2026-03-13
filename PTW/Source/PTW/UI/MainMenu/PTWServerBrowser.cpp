// Fill out your copyright notice in the Description page of Project Settings.


#include "PTWServerBrowser.h"
#include "Components/Border.h"
#include "Components/Button.h"
#include "Components/CheckBox.h"
#include "Components/VerticalBox.h"
#include "Components/EditableText.h"
#include "Components/TextBlock.h"
#include "GameFramework/PlayerState.h"
#include "Kismet/GameplayStatics.h"
#include "PTW/UI/MainMenu/PTWServerListRow.h"
#include "PTW/System/PTWSessionSubsystem.h"
#include "System/Server/GameplayServerTags.h"
#include "System/Server/PTWHTTPRequestManager.h"
#include "System/Server/PTWHTTPRequestTypes.h"
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
	if (!IsValid(HTTPRequestManagerClass))
	{
		HTTPRequestManagerClass = UPTWHTTPRequestManager::StaticClass();
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
		#if WITH_EDITOR
		DevJoinButton->SetVisibility(ESlateVisibility::Visible);
		DevJoinButton->OnClicked.AddDynamic(this, &ThisClass::DevJoinAction);
		#endif
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
	
	if (!IsValid(HTTPRequestManager))
	{
		HTTPRequestManager = NewObject<UPTWHTTPRequestManager>(this, HTTPRequestManagerClass);
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
	// SessionConfig.bIsDedicatedServer = UE_SERVER;
	SessionConfig.bIsDedicatedServer = DedicatedCheckBox->IsChecked();
	
	if (SessionConfig.bIsDedicatedServer)
	{
		// Dedicated Server는 AWS GameLift에서 원격으로 Fleet Instance에 빈 프로세스를 선택하고 GameSession을 생성.
		HTTPRequestManager->CreateGameSession();
	}
	else
	{
		// Listen Server는 현재 Desktop 에서 실행.
		UGameInstance* GameInstance = GetGameInstance();
		if (!IsValid(GameInstance)) return;
	
		UPTWSessionSubsystem* SessionSubsystem = GameInstance->GetSubsystem<UPTWSessionSubsystem>();
		if (!IsValid(SessionSubsystem)) return;
	
		SessionSubsystem->CreateGameSession(SessionConfig);
	}
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

void UPTWServerBrowser::OnClickedTestButton()
{
	/*
	HTTPRequestManager->OnListFleetsResponseReceived.AddUniqueDynamic(this, &ThisClass::OnListFleetsResponseReceived);
	HTTPRequestManager->RequestListFleets();
	TestButton->SetIsEnabled(false);
	*/
	
	HTTPRequestManager->JoinGameSession();
	TestButton->SetIsEnabled(false);
}

void UPTWServerBrowser::OnListFleetsResponseReceived(const struct FPTWListFleetsResponse& ListFleetsResponse,
	bool bwasSuccessful)
{
	if (HTTPRequestManager->OnListFleetsResponseReceived.IsAlreadyBound(this, &ThisClass::OnListFleetsResponseReceived))
	{
		HTTPRequestManager->OnListFleetsResponseReceived.RemoveDynamic(this, &ThisClass::OnListFleetsResponseReceived);
	}
	ServerListVerticalBox->ClearChildren();
	if (bwasSuccessful)
	{
		for (const FString& FleetId : ListFleetsResponse.FleetIds)
		{
			UPTWServerListRow* ServerListRow = CreateWidget<UPTWServerListRow>(this, ServerListRowClass);
			ServerListRow->GetServerName()->SetText(FText::FromString(FleetId));
			ServerListVerticalBox->AddChildToVerticalBox(ServerListRow);
		}
	}
	else
	{
		UPTWServerListRow* ServerListRow = CreateWidget<UPTWServerListRow>(this, ServerListRowClass);
		ServerListRow->GetServerName()->SetText(FText::FromString("Something went wrong!"));
		ServerListVerticalBox->AddChildToVerticalBox(ServerListRow);
	}
	TestButton->SetIsEnabled(true);
	
	if (!IsValid(ServerNameEditableText)) return;
	
	UGameInstance* GameInstance = GetGameInstance();
	if (!IsValid(GameInstance)) return;
	
	UPTWSessionSubsystem* SessionSubsystem = GameInstance->GetSubsystem<UPTWSessionSubsystem>();
	if (!IsValid(SessionSubsystem)) return;
	
	SessionSubsystem->FindGameSession();
}

void UPTWServerBrowser::DevJoinAction()
{
	UGameplayStatics::OpenLevel(GetWorld(), TEXT("127.0.0.1"));
}
#undef LOCTEXT_NAMESPACE
