// Fill out your copyright notice in the Description page of Project Settings.


#include "PTWGameLiftServerSubsystem.h"
#if WITH_GAMELIFT
#include "GameLiftServerSDK.h"
#include "HttpModule.h"
#include "PTWGameLiftClientSubsystem.h"
#include "PTWSteamSessionSubsystem.h"
#include "Interfaces/IHttpResponse.h"
#include "Server/GameplayServerTags.h"
#include "Server/PTWAPIData.h"
#endif

#if WITH_GAMELIFT

UPTWGameLiftServerSubsystem::UPTWGameLiftServerSubsystem()
{
	if (!IsValid(ServerAPIData))
	{
		static ConstructorHelpers::FObjectFinder<UPTWAPIData> DataAssetFinder(TEXT("/Game/_PTW/System/Server/DA_PTW_GameLift_ServerAPI.DA_PTW_GameLift_ServerAPI"));
		if (DataAssetFinder.Succeeded())
		{
			ServerAPIData = DataAssetFinder.Object; 
		}
	}
}

UPTWGameLiftServerSubsystem* UPTWGameLiftServerSubsystem::Get(const UObject* WorldContextObject)
{
	UWorld* World = GEngine->GetWorldFromContextObject(WorldContextObject, EGetWorldErrorMode::LogAndReturnNull);
	if (IsValid(World))
	{
		if (UGameInstance* GI = World->GetGameInstance())
		{
			return GI->GetSubsystem<UPTWGameLiftServerSubsystem>();
		}
	}
	return nullptr;
}

void UPTWGameLiftServerSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);

	MapLoadDelegateHandle = FCoreUObjectDelegates::PostLoadMapWithWorld.AddUObject(this, &ThisClass::OnMapLoaded);
}

void UPTWGameLiftServerSubsystem::Deinitialize()
{
	if (MapLoadDelegateHandle.IsValid())
	{
		FCoreUObjectDelegates::PostLoadMapWithWorld.Remove(MapLoadDelegateHandle);
		MapLoadDelegateHandle.Reset();
	}
	
	Super::Deinitialize();
}

void UPTWGameLiftServerSubsystem::OnMapLoaded(UWorld* LoadedWorld)
{
	if (!LoadedWorld) return;
	if (!IsRunningDedicatedServer()) return;
	
	if (GameLiftSdkModule)
	{
		RequestGameSessionUpdate();
	}
	
	if (MapLoadDelegateHandle.IsValid())
	{
		FCoreUObjectDelegates::PostLoadMapWithWorld.Remove(MapLoadDelegateHandle);
		MapLoadDelegateHandle.Reset();
	}
}

void UPTWGameLiftServerSubsystem::UpdateSessionToReady()
{
	IOnlineSessionPtr SessionInterface = nullptr;
	if (UPTWSteamSessionSubsystem* SteamSessionSubsystem = UPTWSteamSessionSubsystem::Get(this))
	{
		SessionInterface = SteamSessionSubsystem->GetSessionInterface();
	}
	if (!SessionInterface.IsValid()) return;
	
	SessionInterface->ClearOnUpdateSessionCompleteDelegate_Handle(UpdateSessionCompleteDelegateHandle);
	
	FString GameLiftSessionId = GameLiftSdkModule->GetGameSessionId().GetResult();
	if (FOnlineSessionSettings* NewSettings = SessionInterface->GetSessionSettings(NAME_GameSession))
	{
		FString Part1 = GameLiftSessionId.Left(100);
		FString Part2 = GameLiftSessionId.Mid(100);
		NewSettings->Set(FName("GameLiftSessionId_1"), Part1, EOnlineDataAdvertisementType::ViaOnlineService);
		NewSettings->Set(FName("GameLiftSessionId_2"), Part2, EOnlineDataAdvertisementType::ViaOnlineService);
		NewSettings->Set(FName("Status"), TEXT("Ready"), EOnlineDataAdvertisementType::ViaOnlineService);
		
		UpdateSessionCompleteDelegateHandle = SessionInterface->AddOnUpdateSessionCompleteDelegate_Handle(
			FOnUpdateSessionCompleteDelegate::CreateUObject(this, &ThisClass::OnUpdateSessionToReadyComplete);
			
		if (SessionInterface->UpdateSession(NAME_GameSession, *NewSettings, true))
		{
			UE_LOG(LogTemp, Log, TEXT("Session update requested."));
		}
		else
		{
			SessionInterface->ClearOnUpdateSessionCompleteDelegate_Handle(UpdateSessionCompleteDelegateHandle);
			UE_LOG(LogTemp, Warning, TEXT("Failed to start session update."));
		}
	}
}

void UPTWGameLiftServerSubsystem::OnUpdateSessionToReadyComplete(FName SessionName, bool bWasSuccessful)
{
	IOnlineSessionPtr SessionInterface = nullptr;
	if (UPTWSteamSessionSubsystem* SteamSessionSubsystem = UPTWSteamSessionSubsystem::Get(this))
	{
		SessionInterface = SteamSessionSubsystem->GetSessionInterface();
	}
	if (!SessionInterface.IsValid()) return;
	
	SessionInterface->ClearOnUpdateSessionCompleteDelegate_Handle(UpdateSessionCompleteDelegateHandle);
}

void UPTWGameLiftServerSubsystem::UpdateGameSession()
{
	FString GameSessionId = GameLiftSdkModule->GetGameSessionId().GetResult();
	FString SteamId;

	if (UPTWSteamSessionSubsystem* SteamSessionSubsystem = UPTWSteamSessionSubsystem::Get(this))
	{
		SteamId = SteamSessionSubsystem->GetSteamServerID();
	}
	
	UE_LOG(LogTemp, Warning, TEXT("GameSessionId: %s"), *GameSessionId);
	UE_LOG(LogTemp, Warning, TEXT("SteamId: %s"), *SteamId);
	
	if (!SteamId.IsEmpty())
	{
		UE_LOG(LogTemp, Log, TEXT("ReportServerInfoToBackend SteamId Is Valid"));
		TSharedRef<IHttpRequest> Request = FHttpModule::Get().CreateRequest();
		Request->OnProcessRequestComplete().BindUObject(this, &ThisClass::UpdateGameSession_Response);
		
		const FString APIUrl = ServerAPIData->GetAPIEndPoint(GameplayServerTags::GameSessionsAPI::UpdateGameSession);
		Request->SetURL(APIUrl);
		Request->SetVerb(TEXT("POST"));
		Request->SetHeader(TEXT("Content-Type"), TEXT("application/json"));
		
		TMap<FString, FString> Params = {
			{ TEXT("gameSessionId"),	GameSessionId },
			{ TEXT("steamId"),			SteamId }
		};
		
		const FString Content = UPTWGameLiftClientSubsystem::SerializeJsonContent(Params);
		Request->SetContentAsString(Content);
		Request->ProcessRequest();
	}
	else
	{
		// NotSteamId
	}
}

void UPTWGameLiftServerSubsystem::UpdateGameSession_Response(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bWasSuccessful)
{
	if (bWasSuccessful && Response.IsValid() && EHttpResponseCodes::IsOk(Response->GetResponseCode()))
	{
		UE_LOG(LogTemp, Log, TEXT("ReportServerInfoToBackend_Response Successful"));
		UE_LOG(LogTemp, Log, TEXT("Successfully reported Steam ID to Backend."));
		FString GameSessionId = GameLiftSdkModule->GetGameSessionId().GetResult();
		if (!GameSessionId.IsEmpty())
		{
			UpdateSessionToReady();
		}
	}
	else
	{
		UE_LOG(LogTemp, Log, TEXT("ReportServerInfoToBackend_Response Failed"));
		UE_LOG(LogTemp, Error, TEXT("Failed to report Steam ID to Backend."));
	}
}


bool UPTWGameLiftServerSubsystem::AcceptPlayerSession(FString PlayerSessionId)
{
	FGameLiftGenericOutcome Outcome = GameLiftSdkModule->AcceptPlayerSession(PlayerSessionId);
	
	if (Outcome.IsSuccess())
	{
		UE_LOG(LogTemp, Log, TEXT("GameLift PlayerSession Accepted: %s"), *PlayerSessionId);
		return true;
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("Failed to accept GameLift PlayerSession: %s"), *Outcome.GetError().m_errorMessage);
		return false;
	}
}

void UPTWGameLiftServerSubsystem::SetupMapLoadDelegateHandle()
{
	if (!IsRunningDedicatedServer()) return;
	if (MapLoadDelegateHandle.IsValid())
	{
		FCoreUObjectDelegates::PostLoadMapWithWorld.Remove(MapLoadDelegateHandle);
		MapLoadDelegateHandle.Reset();
	}
	MapLoadDelegateHandle = FCoreUObjectDelegates::PostLoadMapWithWorld.AddUObject(this, &ThisClass::OnMapLoaded);
}

void UPTWGameLiftServerSubsystem::RemovePlayerSession(FString PlayerSessionId)
{
	if (GameLiftSdkModule)
	{
		GameLiftSdkModule->RemovePlayerSession(PlayerSessionId);
		UE_LOG(LogTemp, Log, TEXT("GameLift 플레이어 세션 제거 완료: %s"), *PlayerSessionId);
	}
}

void UPTWGameLiftServerSubsystem::ExitGameSession()
{
	if (GameLiftSdkModule)
	{
		GameLiftSdkModule->ProcessEnding();
	}
}

#endif
