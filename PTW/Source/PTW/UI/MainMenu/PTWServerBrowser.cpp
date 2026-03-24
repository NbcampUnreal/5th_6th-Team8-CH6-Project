// Fill out your copyright notice in the Description page of Project Settings.


#include "PTWServerBrowser.h"
#include "Components/Border.h"
#include "Components/Button.h"
#include "Components/CheckBox.h"
#include "Components/VerticalBox.h"
#include "Components/EditableText.h"
#include "CoreFramework/MainMenu/PTWMainMenuPlayerController.h"
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
		#if WITH_EDITOR
		TestButton->SetVisibility(ESlateVisibility::Visible);
		TestButton->OnClicked.AddDynamic(this, &ThisClass::OnClickedTestButton);
		#endif
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
		if (UPTWSessionSubsystem* SessionSubsystem = GameInstance->GetSubsystem<UPTWSessionSubsystem>())
		{
			SessionSubsystem->OnSessionSearchComplete.AddUniqueDynamic(this, &ThisClass::OnFindSessionsComplete);
			SessionSubsystem->OnSteamSessionMessageReceived.AddUniqueDynamic(this, &ThisClass::OnSessionMessageReceived);
		}
		if (UPTWGameLiftSubsystem* GameLiftSubsystem = GameInstance->GetSubsystem<UPTWGameLiftSubsystem>())
		{
			GameLiftSubsystem->OnGameLiftSessionMessageReceived.AddUniqueDynamic(this, &ThisClass::OnSessionMessageReceived);
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
		if (UPTWSessionSubsystem* SessionSubsystem = GameInstance->GetSubsystem<UPTWSessionSubsystem>())
		{
			SessionSubsystem->CreateGameSession(SessionConfig, true);
		}
	}
	else
	{
		// 데디서버로 원격 세션 생성
		if (UPTWGameLiftSubsystem* GameLiftSubsystem = GameInstance->GetSubsystem<UPTWGameLiftSubsystem>())
		{
			GameLiftSubsystem->CreateGameSession(SessionConfig);
		}
	}
}

void UPTWServerBrowser::OnClickedFindServerButton()
{
	SetIsEnabled(false);
	ServerListVerticalBox->ClearChildren();
	
	if (!IsValid(ServerNameEditableText)) return;
	
	UGameInstance* GameInstance = GetGameInstance();
	if (!IsValid(GameInstance)) return;
	
	if (UPTWSessionSubsystem* SessionSubsystem = GameInstance->GetSubsystem<UPTWSessionSubsystem>())
	{
		SessionSubsystem->FindGameSession();
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

void UPTWServerBrowser::OnClickedTestButton()
{
	TestButton->SetIsEnabled(false);
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
