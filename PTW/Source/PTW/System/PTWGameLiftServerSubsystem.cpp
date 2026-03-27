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
		GameLiftSdkModule->ActivateGameSession();
		ReportServerInfoToBackend();
	}
	
	if (MapLoadDelegateHandle.IsValid())
	{
		FCoreUObjectDelegates::PostLoadMapWithWorld.Remove(MapLoadDelegateHandle);
		MapLoadDelegateHandle.Reset();
	}
}

void UPTWGameLiftServerSubsystem::ReportServerInfoToBackend()
{
	// FString GameSessionId = FString(InGameSession.GetGameSessionId());
	FString GameSessionId = GameLiftSdkModule->GetGameSessionId().GetResult();
	FString SteamId;
	
	if (UWorld* World = GetWorld())
	{
		if(UGameInstance* GI = World->GetGameInstance())
		{
			if (UPTWSteamSessionSubsystem* SessionSubsystem = GI->GetSubsystem<UPTWSteamSessionSubsystem>())
			{
				SteamId = SessionSubsystem->GetSteamServerID();
			}
		}
	}
	
	UE_LOG(LogTemp, Warning, TEXT("GameSessionId: %s"), *GameSessionId);
	UE_LOG(LogTemp, Warning, TEXT("SteamId: %s"), *SteamId);
	
	if (!SteamId.IsEmpty())
	{
		UE_LOG(LogTemp, Log, TEXT("ReportServerInfoToBackend SteamId Is Valid"));
		TSharedRef<IHttpRequest> Request = FHttpModule::Get().CreateRequest();
		Request->OnProcessRequestComplete().BindUObject(this, &ThisClass::ReportServerInfoToBackend_Response);
		
		const FString APIUrl = ServerAPIData->GetAPIEndPoint(GameplayServerTags::GameSessionsAPI::ReportServerInfoToBackend);
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

void UPTWGameLiftServerSubsystem::ReportServerInfoToBackend_Response(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bWasSuccessful)
{
	if (bWasSuccessful && Response.IsValid() && EHttpResponseCodes::IsOk(Response->GetResponseCode()))
	{
		UE_LOG(LogTemp, Log, TEXT("ReportServerInfoToBackend_Response Successful"));
		UE_LOG(LogTemp, Log, TEXT("Successfully reported Steam ID to Backend."));
		FString GameSessionId = GameLiftSdkModule->GetGameSessionId().GetResult();
		if (!GameSessionId.IsEmpty())
		{
			if (UPTWSteamSessionSubsystem* SessionSubsystem = UPTWSteamSessionSubsystem::Get(this))
			{
				SessionSubsystem->OnGameSessionActivated(GameSessionId);
			}
		}
	}
	else
	{
		UE_LOG(LogTemp, Log, TEXT("ReportServerInfoToBackend_Response Failed"));
		UE_LOG(LogTemp, Error, TEXT("Failed to report Steam ID to Backend."));
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
