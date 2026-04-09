#include "PTWGameLiftServerSubsystem.h"

#include "Debug/PTWLogCategorys.h"
#include "Server/PTWAPIData.h"
#include "Session/PTWSessionConfig.h"
#include "Utilities/PTWJsonUtility.h"
#if WITH_GAMELIFT
#include "GameLiftServerSDK.h"
#include "HttpModule.h"
#include "PTWSteamSessionSubsystem.h"
#include "Interfaces/IHttpResponse.h"
#include "Server/GameplayServerTags.h"
#endif

UPTWGameLiftServerSubsystem::UPTWGameLiftServerSubsystem()
{
#if WITH_GAMELIFT
	if (!IsValid(ServerAPIData))
	{
		static ConstructorHelpers::FObjectFinder<UPTWAPIData> DataAssetFinder(TEXT("/Game/_PTW/System/Server/DA_PTW_GameLift_ServerAPI.DA_PTW_GameLift_ServerAPI"));
		if (DataAssetFinder.Succeeded())
		{
			ServerAPIData = DataAssetFinder.Object; 
		}
	}
#endif
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

#if WITH_GAMELIFT

IOnlineSessionPtr UPTWGameLiftServerSubsystem::GetSessionInterface() const
{
	if (UPTWSteamSessionSubsystem* SteamSessionSubsystem = UPTWSteamSessionSubsystem::Get(this))
	{
		return SteamSessionSubsystem->GetSessionInterface();
	}
	return nullptr;
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

void UPTWGameLiftServerSubsystem::UpdateSessionToReady()
{
	IOnlineSessionPtr SessionInterface = GetSessionInterface();
	if (!SessionInterface.IsValid()) return;
	
	SessionInterface->ClearOnUpdateSessionCompleteDelegate_Handle(UpdateSessionCompleteDelegateHandle);
	
	UPTWSteamSessionSubsystem* SteamSessionSubsystem = UPTWSteamSessionSubsystem::Get(this);
	FString SteamId = SteamSessionSubsystem->GetSteamServerID();
		
	if (FOnlineSessionSettings* NewSettings = SessionInterface->GetSessionSettings(NAME_GameSession))
	{
		NewSettings->Set(PTWSessionKey::SteamId, SteamId, EOnlineDataAdvertisementType::ViaOnlineService);
		UpdateSessionCompleteDelegateHandle = SessionInterface->AddOnUpdateSessionCompleteDelegate_Handle(
			FOnUpdateSessionCompleteDelegate::CreateUObject(this, &ThisClass::OnUpdateSessionToReadyComplete));
			
		if (SessionInterface->UpdateSession(NAME_GameSession, *NewSettings, true))
		{
			UE_LOG(Log_Steam, Display, TEXT("[게임세션 갱신요청] 스팀게임세션 갱신요청 전송성공"));
		}
		else
		{
			SessionInterface->ClearOnUpdateSessionCompleteDelegate_Handle(UpdateSessionCompleteDelegateHandle);
			UE_LOG(Log_Steam, Error, TEXT("[게임세션 갱신요청] 스팀게임세션 갱신요청 전송실패"));
		}
	}
}

void UPTWGameLiftServerSubsystem::OnUpdateSessionToReadyComplete(FName SessionName, bool bWasSuccessful)
{
	IOnlineSessionPtr SessionInterface = GetSessionInterface();
	if (!SessionInterface.IsValid()) return;
	
	SessionInterface->ClearOnUpdateSessionCompleteDelegate_Handle(UpdateSessionCompleteDelegateHandle);
	
	if (bWasSuccessful)
	{
		UE_LOG(Log_Steam, Display, TEXT("[게임세션 갱신응답] 스팀게임세션 갱신성공 응답"));
		ActivateSessionAndUpdate();
	}
	else
	{
		UE_LOG(Log_Steam, Error, TEXT("[게임세션 갱신응답] 스팀게임세션 갱신실패 응답"));
	}
}

void UPTWGameLiftServerSubsystem::ActivateSessionAndUpdate()
{
	FString GameSessionId = GameLiftSdkModule->GetGameSessionId().GetResult();
	FString SteamId;

	if (UPTWSteamSessionSubsystem* SteamSessionSubsystem = UPTWSteamSessionSubsystem::Get(this))
	{
		SteamId = SteamSessionSubsystem->GetSteamServerID();
	}
	
	if (!SteamId.IsEmpty() && !GameSessionId.IsEmpty())
	{
		TSharedRef<IHttpRequest> Request = FHttpModule::Get().CreateRequest();
		Request->OnProcessRequestComplete().BindUObject(this, &ThisClass::ActivateSessionAndUpdate_Response);
		
		const FString APIUrl = ServerAPIData->GetAPIEndPoint(GameplayServerTags::GameSessionsAPI::ActivateSessionAndUpdate);
		Request->SetURL(APIUrl);
		Request->SetVerb(TEXT("POST"));
		Request->SetHeader(TEXT("Content-Type"), TEXT("application/json"));
		
		TMap<FString, FString> Params = {
			{ TEXT("gameSessionId"),	GameSessionId },
			{ TEXT("steamId"),			SteamId }
		};
		
		const FString Content = UPTWJsonUtility::MakeHTTPRequestBody(Params);
		Request->SetContentAsString(Content);
		if (Request->ProcessRequest())
		{
			UE_LOG(Log_DynamoDB, Display, TEXT("[DB 갱신요청] DynamoDB 갱신요청 전송성공 (SteamId)"));
		}
		else
		{
			UE_LOG(Log_DynamoDB, Error, TEXT("[DB 갱신요청] DynamoDB 갱신요청 전송실패 (SteamId)"));
		}
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("GameSessionId or SteamId 유효하지 않습니다."));
		UE_LOG(Log_DynamoDB, Error, TEXT("[DB 갱신요청] DynamoDB 갱신요청 전송실패 (SteamId)"));
	}
}

void UPTWGameLiftServerSubsystem::ActivateSessionAndUpdate_Response(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bWasSuccessful)
{
	if (bWasSuccessful && Response.IsValid() && EHttpResponseCodes::IsOk(Response->GetResponseCode()))
	{
		UE_LOG(Log_DynamoDB, Display, TEXT("[DB 갱신응답] DynamoDB에 SteamId 갱신성공 응답"));
		UpdateSessionToReady();
	}
	else
	{
		UE_LOG(Log_DynamoDB, Error, TEXT("[DB 갱신응답] DynamoDB에 SteamId 갱신실패 응답"));
	}
}

void UPTWGameLiftServerSubsystem::UpdateSessionState(FString Action)
{
	FString GameSessionId = GameLiftSdkModule->GetGameSessionId().GetResult();
	
	if (!GameSessionId.IsEmpty() && (Action == TEXT("ACTIVE") || Action == TEXT("TERMINATE")))
	{
		TSharedRef<IHttpRequest> Request = FHttpModule::Get().CreateRequest();
		Request->OnProcessRequestComplete().BindUObject(this, &ThisClass::UpdateSessionState_Response, Action);
		
		const FString APIUrl = ServerAPIData->GetAPIEndPoint(GameplayServerTags::GameSessionsAPI::UpdateSessionState);
		Request->SetURL(APIUrl);
		Request->SetVerb(TEXT("POST"));
		Request->SetHeader(TEXT("Content-Type"), TEXT("application/json"));
		
		// Action : ACTIVE / TERMINATE
		TMap<FString, FString> Params = {
			{ TEXT("gameSessionId"),	GameSessionId },
			{ TEXT("action"),			Action }
		};
		
		const FString Content = UPTWJsonUtility::MakeHTTPRequestBody(Params);
		Request->SetContentAsString(Content);
		if (Request->ProcessRequest())
		{
			UE_LOG(Log_DynamoDB, Display, TEXT("[DB 갱신요청] DynamoDB 속성 갱신요청 전송성공 (ServerState: %s)"), *Action);
		}
		else
		{
			UE_LOG(Log_DynamoDB, Error, TEXT("[DB 갱신요청] DynamoDB 속성 갱신요청 전송실패 (ServerState: %s)"), *Action);
		}
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("GameSessionId or Action이 유효하지 않습니다."));
		UE_LOG(Log_DynamoDB, Error, TEXT("[DB 갱신요청] DynamoDB 속성 갱신요청 전송실패 (ServerState: %s)"), *Action);
	}
}

void UPTWGameLiftServerSubsystem::UpdateSessionState_Response(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bWasSuccessful, const FString Action)
{
	if (bWasSuccessful && Response.IsValid() && EHttpResponseCodes::IsOk(Response->GetResponseCode()))
	{
		UE_LOG(Log_DynamoDB, Display, TEXT("[DB 갱신응답] DynamoDB 속성 갱신성공 응답"));
		if (OnUpdateSessionStateCompleted.IsBound())
		{
			OnUpdateSessionStateCompleted.Execute(Action);
		}
	}
	else
	{
		UE_LOG(Log_DynamoDB, Error, TEXT("[DB 갱신응답] DynamoDB 속성 갱신실패 응답"));
	}
}

bool UPTWGameLiftServerSubsystem::AcceptPlayerSession(FString PlayerSessionId)
{
	FGameLiftGenericOutcome Outcome = GameLiftSdkModule->AcceptPlayerSession(PlayerSessionId);
	if (Outcome.IsSuccess())
	{
		UE_LOG(Log_GameLift, Display, TEXT("[플레이어 세션수락] 플레이어세션 접속 수락: %s"), *PlayerSessionId);
		return true;
	}
	else
	{
		UE_LOG(Log_GameLift, Error, TEXT("[플레이어 세션수락] 플레이어세션 접속 거절: %s"), *Outcome.GetError().m_errorMessage);
		return false;
	}
}

void UPTWGameLiftServerSubsystem::UpdatePlayerCount(FString Action)
{
	FString GameSessionId = GameLiftSdkModule->GetGameSessionId().GetResult();
	
	if (!GameSessionId.IsEmpty() && (Action == TEXT("Join") || Action == TEXT("Leave")))
	{
		TSharedRef<IHttpRequest> Request = FHttpModule::Get().CreateRequest();
		Request->OnProcessRequestComplete().BindUObject(this, &ThisClass::UpdatePlayerCount_Response);
		
		const FString APIUrl = ServerAPIData->GetAPIEndPoint(GameplayServerTags::GameSessionsAPI::UpdatePlayerCount);
		Request->SetURL(APIUrl);
		Request->SetVerb(TEXT("POST"));
		Request->SetHeader(TEXT("Content-Type"), TEXT("application/json"));
		
		// Action : Join / Leave
		TMap<FString, FString> Params = {
			{ TEXT("gameSessionId"),	GameSessionId },
			{ TEXT("action"),			Action }
		};
		
		const FString Content = UPTWJsonUtility::MakeHTTPRequestBody(Params);
		Request->SetContentAsString(Content);
		if (Request->ProcessRequest())
		{
			UE_LOG(Log_DynamoDB, Display, TEXT("[DB 갱신요청] DynamoDB 속성갱신 요청성공 (CurrentPlayerCount)"));
		}
		else
		{
			UE_LOG(Log_DynamoDB, Error, TEXT("[DB 갱신요청] DynamoDB 속성갱신 요청실패 (CurrentPlayerCount)"));
		}
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("GameSessionId or Action이 유효하지 않습니다."));
		UE_LOG(Log_DynamoDB, Error, TEXT("[DB 갱신요청] DynamoDB 속성갱신 요청실패 (CurrentPlayerCount)"));
	}
}

void UPTWGameLiftServerSubsystem::UpdatePlayerCount_Response(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bWasSuccessful)
{
	if (bWasSuccessful && Response.IsValid() && EHttpResponseCodes::IsOk(Response->GetResponseCode()))
	{
		UE_LOG(Log_DynamoDB, Display, TEXT("[DB 갱신응답] DynamoDB 속성갱신 성공응답 (CurrentPlayerCount)"));
	}
	else
	{
		UE_LOG(Log_DynamoDB, Error, TEXT("[DB 갱신응답] DynamoDB 속성갱신 실패응답 (CurrentPlayerCount)"));
	}
}

void UPTWGameLiftServerSubsystem::RemovePlayerSession(FString PlayerSessionId)
{
	if (GameLiftSdkModule)
	{
		GameLiftSdkModule->RemovePlayerSession(PlayerSessionId);
		UE_LOG(Log_GameLift, Display, TEXT("[플레이어 세션제거] 플레이어세션 제거: %s"), *PlayerSessionId);
	}
}

void UPTWGameLiftServerSubsystem::ExitGameSession()
{
	if (GameLiftSdkModule)
	{
		GameLiftSdkModule->ProcessEnding();
	}
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
		UpdateSessionToReady();
		GetWorld()->GetTimerManager().SetTimer(UpdateSessionStateTimer, [=, this]()
		{
			UpdateSessionState("ACTIVE");
		},
		60.0f, true);
	}
	
	if (MapLoadDelegateHandle.IsValid())
	{
		FCoreUObjectDelegates::PostLoadMapWithWorld.Remove(MapLoadDelegateHandle);
		MapLoadDelegateHandle.Reset();
	}
}

#endif
