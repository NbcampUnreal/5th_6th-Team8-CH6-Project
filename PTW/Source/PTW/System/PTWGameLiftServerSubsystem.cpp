// Fill out your copyright notice in the Description page of Project Settings.


#include "PTWGameLiftServerSubsystem.h"
#include "Server/PTWAPIData.h"
#include "Session/PTWSessionConfig.h"
#if WITH_GAMELIFT
#include "GameLiftServerSDK.h"
#include "HttpModule.h"
#include "PTWGameLiftClientSubsystem.h"
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

#if WITH_GAMELIFT

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

IOnlineSessionPtr UPTWGameLiftServerSubsystem::GetSessionInterface() const
{
	if (UPTWSteamSessionSubsystem* SteamSessionSubsystem = UPTWSteamSessionSubsystem::Get(this))
	{
		return SteamSessionSubsystem->GetSessionInterface();
	}
	
	UE_LOG(LogTemp, Warning, TEXT("SessionInterface를 불러오는데 실패하였습니다."));
	return nullptr;
}

void UPTWGameLiftServerSubsystem::OnMapLoaded(UWorld* LoadedWorld)
{
	if (!LoadedWorld) return;
	if (!IsRunningDedicatedServer()) return;
	
	if (GameLiftSdkModule)
	{
		UpdateGameSession();
	}
	
	if (MapLoadDelegateHandle.IsValid())
	{
		FCoreUObjectDelegates::PostLoadMapWithWorld.Remove(MapLoadDelegateHandle);
		MapLoadDelegateHandle.Reset();
	}
}

void UPTWGameLiftServerSubsystem::UpdateSessionToReady()
{
	IOnlineSessionPtr SessionInterface = GetSessionInterface();
	if (!SessionInterface.IsValid()) return;
	
	SessionInterface->ClearOnUpdateSessionCompleteDelegate_Handle(UpdateSessionCompleteDelegateHandle);
	
	FString GameLiftSessionId = GameLiftSdkModule->GetGameSessionId().GetResult();
	if (FOnlineSessionSettings* NewSettings = SessionInterface->GetSessionSettings(NAME_GameSession))
	{
		FString RefindSeesionId = GameLiftSessionId.Replace(TEXT("arn:aws:gamelift:"), TEXT(""));
		RefindSeesionId = RefindSeesionId.Replace(TEXT("::gamesession/"), TEXT("|"));
		
		NewSettings->Set(PTWSessionKey::GameLiftSessionId, RefindSeesionId, EOnlineDataAdvertisementType::ViaOnlineService);
		NewSettings->Set(PTWSessionKey::JOINABLE, true, EOnlineDataAdvertisementType::ViaOnlineService);
		
		UpdateSessionCompleteDelegateHandle = SessionInterface->AddOnUpdateSessionCompleteDelegate_Handle(
			FOnUpdateSessionCompleteDelegate::CreateUObject(this, &ThisClass::OnUpdateSessionToReadyComplete));
			
		if (SessionInterface->UpdateSession(NAME_GameSession, *NewSettings, true))
		{
			UE_LOG(LogTemp, Display, TEXT("[Steam-MasterServer Request] 스팀세션 [준비됨] 상태로 업데이트 요청 전송을 완료했습니다."));
		}
		else
		{
			SessionInterface->ClearOnUpdateSessionCompleteDelegate_Handle(UpdateSessionCompleteDelegateHandle);
			UE_LOG(LogTemp, Display, TEXT("[Steam-MasterServer Request] 스팀세션 [준비됨] 상태로 업데이트 요청 전송을 실패하였습니다."));
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
		UE_LOG(LogTemp, Display, TEXT("[Steam-MasterServer Response] 스팀 게임세션을 [준비됨] 상태로 업데이트 완료하였습니다."));
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("[Steam-MasterServer Response] 스팀 게임세션을 [준비됨] 상태로 업데이트 실패하였습니다."));
	}
}

void UPTWGameLiftServerSubsystem::UpdateGameSession()
{
	FString GameSessionId = GameLiftSdkModule->GetGameSessionId().GetResult();
	FString SteamId;

	if (UPTWSteamSessionSubsystem* SteamSessionSubsystem = UPTWSteamSessionSubsystem::Get(this))
	{
		SteamId = SteamSessionSubsystem->GetSteamServerID();
	}
	
	UE_LOG(LogTemp, Display, TEXT("현재: GameSessionId: %s"), *GameSessionId);
	UE_LOG(LogTemp, Display, TEXT("현재: SteamId: %s"), *SteamId);
	
	if (!SteamId.IsEmpty() && !GameSessionId.IsEmpty())
	{
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
		if (Request->ProcessRequest())
		{
			UE_LOG(LogTemp, Display, TEXT("[GameLift-MasterServer Request] 게임리프트 게임세션에 SteamId 업데이트 요청 전송을 완료했습니다."));
		}
		else
		{
			UE_LOG(LogTemp, Error, TEXT("[GameLift-MasterServer Request] 게임리프트 게임세션에 SteamId 업데이트 요청 전송을 실패했습니다"));
		}
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("GameSessionId 또는 SteamId가 유효하지 않습니다."));
	}
}

void UPTWGameLiftServerSubsystem::UpdateGameSession_Response(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bWasSuccessful)
{
	if (bWasSuccessful && Response.IsValid() && EHttpResponseCodes::IsOk(Response->GetResponseCode()))
	{
		UE_LOG(LogTemp, Display, TEXT("[GameLift-MasterServer Response] 게임리프트 게임세션에 SteamId 업데이트를 완료했습니다."));
		UpdateSessionToReady();
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("[GameLift-MasterServer Response] 게임리프트 게임세션에 SteamId 업데이트를 실패했습니다."));
	}
}


bool UPTWGameLiftServerSubsystem::AcceptPlayerSession(FString PlayerSessionId)
{
	FGameLiftGenericOutcome Outcome = GameLiftSdkModule->AcceptPlayerSession(PlayerSessionId);
	
	if (Outcome.IsSuccess())
	{
		UE_LOG(LogTemp, Log, TEXT("게임리프트 플레이어세션 접속 수락: %s"), *PlayerSessionId);
		return true;
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("게임리프트 플레이어세션 접속 실패: %s"), *Outcome.GetError().m_errorMessage);
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
		UE_LOG(LogTemp, Log, TEXT("게임리프트 플레이어 세션 제거 완료: %s"), *PlayerSessionId);
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
