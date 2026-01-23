// Fill out your copyright notice in the Description page of Project Settings.


#include "PTWLobbyBrowser.h"
#include "Components/Border.h"
#include "Components/Button.h"
#include "Components/VerticalBox.h"
#include "Components/EditableText.h"
#include "PTW/UI/MainMenu/PTWLobbyListRow.h"
#include "PTW/System/PTWSessionSubsystem.h"

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
			SessionSubsystem->OnFindLobbiesCompleteDelegate.AddDynamic(this, &ThisClass::OnFindLobbiesComplete);
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
	if (!IsValid(LobbyMenuBorder))
	{
		return;
	}
	
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
	FLobbySettings SessionData;
	if (IsValid(LobbyNameEditableText))
	{
		SessionData.LobbyName = LobbyNameEditableText->GetText().ToString();
	}
	
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
	SessionSubsystem->CreateLobbySession(SessionData);
}

void UPTWLobbyBrowser::OnClickedFindLobbyButton()
{
	if (!IsValid(LobbyNameEditableText))
	{
		return;
	}
	LobbyListVerticalBox->ClearChildren();
	
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
	
	SessionSubsystem->FindLobbySession();
	
	
}

void UPTWLobbyBrowser::OnFindLobbiesComplete(const TArray<FBlueprintSessionResult>& SessionResults)
{
	if (SessionResults.Num() <= 0)
	{
		return;
	}
	for (const FBlueprintSessionResult& SessionResult : SessionResults)
	{
		UPTWLobbyListRow* LobbyListRow = CreateWidget<UPTWLobbyListRow>(this, LobbyListRowClass);
		LobbyListRow->Setup(SessionResult);
		LobbyListVerticalBox->AddChildToVerticalBox(LobbyListRow);
	}
}
