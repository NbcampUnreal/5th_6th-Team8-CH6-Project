// Fill out your copyright notice in the Description page of Project Settings.


#include "PTWServerBrowser.h"

#include "PTWMainMenu.h"
#include "Components/Border.h"
#include "Components/Button.h"
#include "Components/CheckBox.h"
#include "Components/VerticalBox.h"
#include "Components/EditableText.h"
#include "CoreFramework/MainMenu/PTWMainMenuPlayerController.h"
#include "GameFramework/PlayerState.h"
#include "Kismet/GameplayStatics.h"
#include "PTW/UI/MainMenu/PTWServerListRow.h"
#include "PTW/System/PTWSteamSessionSubsystem.h"
#include "System/PTWGameLiftClientSubsystem.h"
#include "System/Session/PTWSessionConfig.h"
#include "UI/PTWUISubsystem.h"

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
		BackButton->OnClicked.AddUniqueDynamic(this, &ThisClass::OnClickedBackButton);	
	}
	
	if (IsValid(ServerMenuButton))
	{
		ServerMenuButton->OnClicked.AddUniqueDynamic(this, &ThisClass::OnClickedServerMenuButton);
	}
	
	if (IsValid(CreateServerButton))
	{
		CreateServerButton->OnClicked.AddUniqueDynamic(this, &ThisClass::OnClickedCreateServerButton);
	}
	
	if (IsValid(FindServerButton))
	{
		FindServerButton->OnClicked.AddUniqueDynamic(this, &ThisClass::OnClickedFindServerButton);
	}
	
	if (IsValid(QuickMatchButton))
	{
		QuickMatchButton->OnClicked.AddUniqueDynamic(this, &ThisClass::OnClickedQuickMatchButton);
	}
	
	if (IsValid(ShortRoundButton))
	{
		ShortRoundButton->OnClicked.AddUniqueDynamic(this, &ThisClass::OnClickedShortRoundButton);
		OnClickedShortRoundButton();
	}
	
	if (IsValid(LongRoundButton))
	{
		LongRoundButton->OnClicked.AddUniqueDynamic(this, &ThisClass::OnClickedLongRoundButton);
	}
	
	if (IsValid(DevJoinButton))
	{
		#if WITH_EDITOR
		DevJoinButton->SetVisibility(ESlateVisibility::Visible);
		DevJoinButton->OnClicked.AddUniqueDynamic(this, &ThisClass::DevJoinAction);
		#endif
	}
	
	UGameInstance* GameInstance = GetGameInstance();
	if (IsValid(GameInstance))
	{
		if (UPTWSteamSessionSubsystem* SteamSessionSubsystem = UPTWSteamSessionSubsystem::Get(this))
		{
			SteamSessionSubsystem->OnSessionSearchComplete.AddUniqueDynamic(this, &ThisClass::OnFindSessionsComplete);
			SteamSessionSubsystem->OnSteamSessionMessageReceived.AddUniqueDynamic(this, &ThisClass::OnSessionMessageReceived);
		}
		if (UPTWGameLiftClientSubsystem* GameLiftClientSubsystem = UPTWGameLiftClientSubsystem::Get(this))
		{
			GameLiftClientSubsystem->OnGameLiftSessionMessageReceived.AddUniqueDynamic(this, &ThisClass::OnSessionMessageReceived);
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
	
	SetIsEnabled(true);
	
	if (IsValid(ServerMenuBorder))
	{
		ServerMenuBorder->SetVisibility(ESlateVisibility::Collapsed);
	}
	
	ServerMenuButton->SetVisibility(ESlateVisibility::Collapsed);
	FindServerButton->SetVisibility(ESlateVisibility::Collapsed);
}

void UPTWServerBrowser::NativeDestruct()
{
	Super::NativeDestruct();
}

void UPTWServerBrowser::OnClickedBackButton()
{
	if (IsValid(MainMenuClass))
	{
		if (ULocalPlayer* LP = GetOwningLocalPlayer())
		{
			if (UPTWUISubsystem* UISubsystem = LP->GetSubsystem<UPTWUISubsystem>())
			{
				UISubsystem->HideSystemWidget(GetClass());
				UISubsystem->ShowSystemWidget(MainMenuClass);
			}
		}
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
	
	SetIsEnabled(false);
	
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
	
	if (!SessionConfig.bIsDedicatedServer)
	{
		// 리슨서버로 세션 생성
		if (UPTWSteamSessionSubsystem* SteamSessionSubsystem = UPTWSteamSessionSubsystem::Get(this))
		{
			SteamSessionSubsystem->CreateGameSession(SessionConfig, true);
		}
	}
	else
	{
		// 데디서버로 원격 세션 생성
		if (UPTWGameLiftClientSubsystem* GameLiftClientSubsystem = UPTWGameLiftClientSubsystem::Get(this))
		{
			GameLiftClientSubsystem->CreateGameSession(SessionConfig);
		}
	}
}

void UPTWServerBrowser::OnClickedFindServerButton()
{
	SetIsEnabled(false);
	ServerListVerticalBox->ClearChildren();
	
	if (!IsValid(ServerNameEditableText)) return;

	if (UPTWSteamSessionSubsystem* SteamSessionSubsystem = UPTWSteamSessionSubsystem::Get(this))
	{
		SteamSessionSubsystem->FindGameSession();
	}
}

void UPTWServerBrowser::OnClickedQuickMatchButton()
{
	if (UPTWGameLiftClientSubsystem* GameLiftClientSubsystem = UPTWGameLiftClientSubsystem::Get(this))
	{
		GameLiftClientSubsystem->SearchQuickSession();
		SetIsEnabled(false);
	}
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
	for (const FOnlineSessionSearchResultBP& SearchResult : SearchResults)
	{
		UPTWServerListRow* ServerListRow = CreateWidget<UPTWServerListRow>(this, ServerListRowClass);
		ServerListRow->SetupSessionMinimalInfo(SearchResult);
		ServerListVerticalBox->AddChildToVerticalBox(ServerListRow);
	}
	SetIsEnabled(true);
}

void UPTWServerBrowser::DevJoinAction()
{
	UGameplayStatics::OpenLevel(GetWorld(), TEXT("127.0.0.1:7777"));
}

void UPTWServerBrowser::OnSessionMessageReceived(const FText& Message)
{
	if (GetIsEnabled() == false)
	{
		SetIsEnabled(true);
	}
	if (IsValid(GetOwningPlayer()))
	{
		if (APTWMainMenuPlayerController* PC = Cast<APTWMainMenuPlayerController>(GetOwningPlayer()))
		{
			PC->Popup(Message);
		}
	}
}

#undef LOCTEXT_NAMESPACE
