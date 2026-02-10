// Fill out your copyright notice in the Description page of Project Settings.


#include "PTWLobbyBrowser.h"

#include "BlueprintDataDefinitions.h"
#include "Components/Border.h"
#include "Components/Button.h"
#include "Components/VerticalBox.h"
#include "Components/EditableText.h"
#include "PTW/UI/MainMenu/PTWLobbyListRow.h"
#include "PTW/System/PTWSessionSubsystem.h"
#include "System/Session/SessionConfig.h"

void UPTWLobbyBrowser::NativeConstruct()
{
	Super::NativeConstruct();
	
	if (!IsValid(LobbyListRowClass))
	{
		LobbyListRowClass = UPTWLobbyListRow::StaticClass();
	}
	
	if (IsValid(BackButton))
	{
		BackButton->OnClicked.AddDynamic(this, &ThisClass::OnClickedBackButton);	
	}
	
	if (IsValid(LobbyMenuButton))
	{
		LobbyMenuButton->OnClicked.AddDynamic(this, &ThisClass::OnClickedLobbyMenuButton);
	}
	
	if (IsValid(CreateLobbyButton))
	{
		CreateLobbyButton->OnClicked.AddDynamic(this, &ThisClass::OnClickedCreateLobbyButton);
	}
	
	if (IsValid(FindLobbyButton))
	{
		FindLobbyButton->OnClicked.AddDynamic(this, &ThisClass::OnClickedFindLobbyButton);
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
	
	if (IsValid(LobbyNameEditableText))
	{
		APlayerState* PS = GetOwningPlayerState();
		if (IsValid(PS))
		{
			FText NewLobbyName = FText::FromString(FString::Printf(TEXT("%s의 서버"), *PS->GetPlayerName()));
			LobbyNameEditableText->SetText(NewLobbyName);
		}
	}
}

void UPTWLobbyBrowser::NativeDestruct()
{
	Super::NativeDestruct();
	
	if (IsValid(BackButton))
	{
		BackButton->OnClicked.RemoveDynamic(this, &ThisClass::OnClickedBackButton);	
	}
}

void UPTWLobbyBrowser::OnClickedBackButton()
{
	if (OnLobbyBackAction.IsBound())
	{
		OnLobbyBackAction.Broadcast();
	}
}

void UPTWLobbyBrowser::OnClickedLobbyMenuButton()
{
	if (!IsValid(LobbyMenuBorder)) return;
	
	if (LobbyMenuBorder->GetVisibility() == ESlateVisibility::Collapsed)
	{
		// UI 숨김 -> 보임
		LobbyMenuBorder->SetVisibility(ESlateVisibility::Visible);
	}
	else
	{
		// UI 보임 -> 숨김
		LobbyMenuBorder->SetVisibility(ESlateVisibility::Collapsed);
	}
}

void UPTWLobbyBrowser::OnClickedCreateLobbyButton()
{
	FSessionConfig SessionConfig;
	if (IsValid(LobbyNameEditableText))
	{
		SessionConfig.ServerName = LobbyNameEditableText->GetText().ToString();
	}
	
	UGameInstance* GameInstance = GetGameInstance();
	if (!IsValid(GameInstance)) return;
	
	UPTWSessionSubsystem* SessionSubsystem = GameInstance->GetSubsystem<UPTWSessionSubsystem>();
	if (!IsValid(SessionSubsystem)) return;
	
	SessionSubsystem->CreateGameSession(SessionConfig);
	// SessionSubsystem->LaunchDedicatedServer(LobbySettings, 16, false);
}

void UPTWLobbyBrowser::OnClickedFindLobbyButton()
{
	LobbyListVerticalBox->ClearChildren();
	
	if (!IsValid(LobbyNameEditableText)) return;
	
	UGameInstance* GameInstance = GetGameInstance();
	if (!IsValid(GameInstance)) return;
	
	UPTWSessionSubsystem* SessionSubsystem = GameInstance->GetSubsystem<UPTWSessionSubsystem>();
	if (!IsValid(SessionSubsystem)) return;
	
	SessionSubsystem->FindGameSession();
}

void UPTWLobbyBrowser::OnFindSessionsComplete(const TArray<FBlueprintSessionResult>& SearchResults)
{
	if (SearchResults.IsEmpty()) return;
	
	for (const FBlueprintSessionResult& SearchResult : SearchResults)
	{
		UPTWLobbyListRow* LobbyListRow = CreateWidget<UPTWLobbyListRow>(this, LobbyListRowClass);
		LobbyListRow->Setup(SearchResult);
		LobbyListVerticalBox->AddChildToVerticalBox(LobbyListRow);
	}
}
