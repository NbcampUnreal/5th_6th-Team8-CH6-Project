// Fill out your copyright notice in the Description page of Project Settings.


#include "PTWServerEntryGameMode.h"
#include "CoreFramework/Game/GameSession/PTWGameSession.h"
#include "Kismet/GameplayStatics.h"
#include "System/PTWGameLiftSubsystem.h"
#include "System/PTWSessionSubsystem.h"
#include "System/Session/PTWSessionConfig.h"

DEFINE_LOG_CATEGORY(GameServerLog);

APTWServerEntryGameMode::APTWServerEntryGameMode()
{
	bUseSeamlessTravel = true;
	GameSessionClass = APTWGameSession::StaticClass();
}

void APTWServerEntryGameMode::BeginPlay()
{
	Super::BeginPlay();
	
#if WITH_GAMELIFT
	InitGameLift();
#endif
}

#if WITH_GAMELIFT
void APTWServerEntryGameMode::InitGameLift()
{
	UE_LOG(GameServerLog, Log, TEXT("Calling InitGameLift..."));
	
	if (FParse::Param(FCommandLine::Get(), *PTWSessionKey::NoGameLift.ToString()))
	{
		if (UPTWSessionSubsystem* SessionSubsystem = GetGameInstance()->GetSubsystem<UPTWSessionSubsystem>())
		{
			FPTWSessionConfig SessionConfig;
			SessionConfig.ServerName = TEXT("NonGameLiftServer");
			SessionConfig.MaxPlayers = 16;
			SessionConfig.bIsDedicatedServer = UE_SERVER;
		
			const TCHAR* CommandLine = FCommandLine::Get();
		
			FString ServerName_cmd = FString::Printf(TEXT("-%s="), *PTWSessionKey::ServerName.ToString());
			FString MaxPlayers_cmd = FString::Printf(TEXT("-%s="), *PTWSessionKey::MaxPlayers.ToString());
		
			FParse::Value(CommandLine, *ServerName_cmd, SessionConfig.ServerName);
			FParse::Value(CommandLine, *MaxPlayers_cmd, SessionConfig.MaxPlayers);
			
			SessionSubsystem->CreateGameSession(SessionConfig, true);
			// FTimerHandle TempTimerHandle;
			// GetWorld()->GetTimerManager().SetTimer(TempTimerHandle, [=, this]()
			// {
			// 	SessionSubsystem->OnGameSessionActivated("1234");
			// }, 10.0f, false);
			return;
		}
	}
	
	// Getting the module first.
	// 먼저 모듈을 가져옵니다.
    FGameLiftServerSDKModule* GameLiftSdkModule = &FModuleManager::LoadModuleChecked<FGameLiftServerSDKModule>(FName("GameLiftServerSDK"));

    //Define the server parameters for a GameLift Anywhere fleet. These are not needed for a GameLift managed EC2 fleet.
	// GameLift Anywhere 플릿을 위한 서버 파라미터를 정의합니다. (GameLift 매니지드 EC2 플릿에서는 필요하지 않습니다.)
	FServerParameters ServerParametersForAnywhere;

    bool bIsAnywhereActive = false;
    if (FParse::Param(FCommandLine::Get(), TEXT("glAnywhere")))
    {
        bIsAnywhereActive = true;
    }

    if (bIsAnywhereActive)
    {
        UE_LOG(GameServerLog, Log, TEXT("Configuring server parameters for Anywhere..."));

        // If GameLift Anywhere is enabled, parse command line arguments and pass them in the ServerParameters object.
    	// GameLift Anywhere가 활성화되어 있으면, 커맨드라인 인자를 파싱해서 ServerParameters 객체에 담아 전달합니다.
    	FString glAnywhereWebSocketUrl = "";
        if (FParse::Value(FCommandLine::Get(), TEXT("glAnywhereWebSocketUrl="), glAnywhereWebSocketUrl))
        {
            ServerParametersForAnywhere.m_webSocketUrl = TCHAR_TO_UTF8(*glAnywhereWebSocketUrl);
        }

        FString glAnywhereFleetId = "";
        if (FParse::Value(FCommandLine::Get(), TEXT("glAnywhereFleetId="), glAnywhereFleetId))
        {
            ServerParametersForAnywhere.m_fleetId = TCHAR_TO_UTF8(*glAnywhereFleetId);
        }

        FString glAnywhereProcessId = "";
        if (FParse::Value(FCommandLine::Get(), TEXT("glAnywhereProcessId="), glAnywhereProcessId))
        {
            ServerParametersForAnywhere.m_processId = TCHAR_TO_UTF8(*glAnywhereProcessId);
        }
        else
        {
            // If no ProcessId is passed as a command line argument, generate a randomized unique string.
        	// 커맨드라인 인자로 ProcessId가 전달되지 않으면, 랜덤한(유일한) 문자열을 생성합니다.
            FString TimeString = FString::FromInt(std::time(nullptr));
            FString ProcessId = "ProcessId_" + TimeString;
            ServerParametersForAnywhere.m_processId = TCHAR_TO_UTF8(*ProcessId);
        }

        FString glAnywhereHostId = "";
        if (FParse::Value(FCommandLine::Get(), TEXT("glAnywhereHostId="), glAnywhereHostId))
        {
            ServerParametersForAnywhere.m_hostId = TCHAR_TO_UTF8(*glAnywhereHostId);
        }

        FString glAnywhereAuthToken = "";
        if (FParse::Value(FCommandLine::Get(), TEXT("glAnywhereAuthToken="), glAnywhereAuthToken))
        {
            ServerParametersForAnywhere.m_authToken = TCHAR_TO_UTF8(*glAnywhereAuthToken);
        }

        FString glAnywhereAwsRegion = "";
        if (FParse::Value(FCommandLine::Get(), TEXT("glAnywhereAwsRegion="), glAnywhereAwsRegion))
        {
            ServerParametersForAnywhere.m_awsRegion = TCHAR_TO_UTF8(*glAnywhereAwsRegion);
        }

        FString glAnywhereAccessKey = "";
        if (FParse::Value(FCommandLine::Get(), TEXT("glAnywhereAccessKey="), glAnywhereAccessKey))
        {
            ServerParametersForAnywhere.m_accessKey = TCHAR_TO_UTF8(*glAnywhereAccessKey);
        }

        FString glAnywhereSecretKey = "";
        if (FParse::Value(FCommandLine::Get(), TEXT("glAnywhereSecretKey="), glAnywhereSecretKey))
        {
            ServerParametersForAnywhere.m_secretKey = TCHAR_TO_UTF8(*glAnywhereSecretKey);
        }

        FString glAnywhereSessionToken = "";
        if (FParse::Value(FCommandLine::Get(), TEXT("glAnywhereSessionToken="), glAnywhereSessionToken))
        {
            ServerParametersForAnywhere.m_sessionToken = TCHAR_TO_UTF8(*glAnywhereSessionToken);
        }

        UE_LOG(GameServerLog, SetColor, TEXT("%s"), COLOR_YELLOW);
        UE_LOG(GameServerLog, Log, TEXT(">>>> WebSocket URL: %s"), *ServerParametersForAnywhere.m_webSocketUrl);
        UE_LOG(GameServerLog, Log, TEXT(">>>> Fleet ID: %s"), *ServerParametersForAnywhere.m_fleetId);
        UE_LOG(GameServerLog, Log, TEXT(">>>> Process ID: %s"), *ServerParametersForAnywhere.m_processId);
        UE_LOG(GameServerLog, Log, TEXT(">>>> Host ID (Compute Name): %s"), *ServerParametersForAnywhere.m_hostId);
        UE_LOG(GameServerLog, Log, TEXT(">>>> Auth Token: %s"), *ServerParametersForAnywhere.m_authToken);
        UE_LOG(GameServerLog, Log, TEXT(">>>> Aws Region: %s"), *ServerParametersForAnywhere.m_awsRegion);
        UE_LOG(GameServerLog, Log, TEXT(">>>> Access Key: %s"), *ServerParametersForAnywhere.m_accessKey);
        UE_LOG(GameServerLog, Log, TEXT(">>>> Secret Key: %s"), *ServerParametersForAnywhere.m_secretKey);
        UE_LOG(GameServerLog, Log, TEXT(">>>> Session Token: %s"), *ServerParametersForAnywhere.m_sessionToken);
        UE_LOG(GameServerLog, SetColor, TEXT("%s"), COLOR_NONE);
    }

    UE_LOG(GameServerLog, Log, TEXT("Initializing the GameLift Server..."));

    //InitSDK will establish a local connection with GameLift's agent to enable further communication.
	// InitSDK는 GameLift 에이전트와 로컬 연결을 설정하여, 이후 통신을 가능하게 합니다.
    FGameLiftGenericOutcome InitSdkOutcome = GameLiftSdkModule->InitSDK(ServerParametersForAnywhere);
    if (InitSdkOutcome.IsSuccess())
    {
        UE_LOG(GameServerLog, SetColor, TEXT("%s"), COLOR_GREEN);
        UE_LOG(GameServerLog, Log, TEXT("GameLift InitSDK succeeded!"));
        UE_LOG(GameServerLog, SetColor, TEXT("%s"), COLOR_NONE);
    }
    else
    {
        UE_LOG(GameServerLog, SetColor, TEXT("%s"), COLOR_RED);
        UE_LOG(GameServerLog, Log, TEXT("ERROR: InitSDK failed : ("));
        FGameLiftError GameLiftError = InitSdkOutcome.GetError();
        UE_LOG(GameServerLog, Log, TEXT("ERROR: %s"), *GameLiftError.m_errorMessage);
        UE_LOG(GameServerLog, SetColor, TEXT("%s"), COLOR_NONE);
        return;
    }

    ProcessParameters = MakeShared<FProcessParameters>();

    //When a game session is created, Amazon GameLift Servers sends an activation request to the game server and passes along the game session object containing game properties and other settings.
    //Here is where a game server should take action based on the game session object.
    //Once the game server is ready to receive incoming player connections, it should invoke GameLiftServerAPI.ActivateGameSession()
	// 게임 세션이 생성되면, Amazon GameLift Servers가 게임 서버로 활성화 요청(activation request)을 보내고
	// 게임 속성 및 기타 설정이 들어있는 GameSession 객체를 함께 전달합니다.
	// 여기에서 게임 서버는 GameSession 객체를 바탕으로 필요한 동작을 수행해야 합니다.
	// 플레이어 접속을 받을 준비가 되면, GameLiftServerAPI.ActivateGameSession()을 호출해야 합니다.
	ProcessParameters->OnStartGameSession.BindLambda([=, this](Aws::GameLift::Server::Model::GameSession InGameSession)
	{
        FString GameSessionId = FString(InGameSession.GetGameSessionId());
        UE_LOG(GameServerLog, Log, TEXT("GameSession Initializing: %s"), *GameSessionId);
		
		UGameInstance* GI = GetGameInstance();
		if (!IsValid(GI)) return;
		
		UPTWGameLiftSubsystem* GameLiftSubsystem = GI->GetSubsystem<UPTWGameLiftSubsystem>();
		if (!IsValid(GameLiftSubsystem)) return;
		
		// GameLiftSdkModule->ActivateGameSession();
		GameLiftSubsystem->SetupMapLoadDelegateHandle();
		GameLiftSubsystem->SetGameLiftSdkModule(GameLiftSdkModule);
		
		AsyncTask(ENamedThreads::GameThread, [=, this]()
		{	
			UPTWSessionSubsystem* SessionSubsystem = GI->GetSubsystem<UPTWSessionSubsystem>();
			FPTWSessionConfig SessionConfig;
			SessionConfig.bIsDedicatedServer = true;
			SessionSubsystem->CreateGameSession(SessionConfig, true);
		});
	});
    //OnProcessTerminate callback. Amazon GameLift Servers will invoke this callback before shutting down an instance hosting this game server.
    //It gives this game server a chance to save its state, communicate with services, etc., before being shut down.
    //In this case, we simply tell Amazon GameLift Servers we are indeed going to shutdown.
	// OnProcessTerminate 콜백. Amazon GameLift Servers는 이 게임 서버를 호스팅 중인 인스턴스를 종료하기 전에 이 콜백을 호출합니다.
	// 게임 서버가 종료되기 전에 상태를 저장하거나, 서비스에 알리거나 하는 등의 작업을 할 기회를 줍니다.
	// 여기서는 단순히 Amazon GameLift Servers에 “종료하겠다”는 것을 알립니다.
    ProcessParameters->OnTerminate.BindLambda([=]()
        {
            UE_LOG(GameServerLog, Log, TEXT("Game Server Process is terminating"));
    	
            // First call ProcessEnding()
    		// 먼저 ProcessEnding()을 호출합니다.
            FGameLiftGenericOutcome processEndingOutcome = GameLiftSdkModule->ProcessEnding();
    	
            // Then call Destroy() to free the SDK from memory
    		// 그 다음 Destroy()를 호출하여 SDK를 메모리에서 해제합니다.
            FGameLiftGenericOutcome destroyOutcome = GameLiftSdkModule->Destroy();
    	
            // Exit the process with success or failure
    		// 성공/실패 여부에 따라 프로세스를 종료합니다.
            if (processEndingOutcome.IsSuccess() && destroyOutcome.IsSuccess()) {
                UE_LOG(GameServerLog, Log, TEXT("Server process ending successfully"));
            }
            else {
                if (!processEndingOutcome.IsSuccess()) {
                    const FGameLiftError& error = processEndingOutcome.GetError();
                    UE_LOG(GameServerLog, Error, TEXT("ProcessEnding() failed. Error: %s"),
                    error.m_errorMessage.IsEmpty() ? TEXT("Unknown error") : *error.m_errorMessage);
                }
                if (!destroyOutcome.IsSuccess()) {
                    const FGameLiftError& error = destroyOutcome.GetError();
                    UE_LOG(GameServerLog, Error, TEXT("Destroy() failed. Error: %s"),
                    error.m_errorMessage.IsEmpty() ? TEXT("Unknown error") : *error.m_errorMessage);
                }
            }
    		FGenericPlatformMisc::RequestExit(false);
        });
         
    //This is the HealthCheck callback.
    //Amazon GameLift Servers will invoke this callback every 60 seconds or so.
    //Here, a game server might want to check the health of dependencies and such.
    //Simply return true if healthy, false otherwise.
    //The game server has 60 seconds to respond with its health status. Amazon GameLift Servers will default to 'false' if the game server doesn't respond in time.
    //In this case, we're always healthy!
	// 이것은 HealthCheck 콜백입니다.
	// Amazon GameLift Servers는 약 60초마다 이 콜백을 호출합니다.
	// 여기에서 게임 서버는 의존성(외부 서비스 등)의 상태를 확인하는 등의 헬스 체크를 수행할 수 있습니다.
	// 정상이라면 true, 비정상이라면 false를 반환하면 됩니다.
	// 게임 서버는 60초 안에 헬스 상태를 응답해야 합니다.
	// 제한 시간 내에 응답하지 않으면 Amazon GameLift Servers는 기본값으로 'false'로 처리합니다.
	// 여기서는 항상 정상(healthy)으로 처리합니다!
    ProcessParameters->OnHealthCheck.BindLambda([]()
        {
            UE_LOG(GameServerLog, Log, TEXT("Performing Health Check"));
            return true;
        });

    //GameServer.exe -port=7777 LOG=server.mylog
    ProcessParameters->port = FURL::UrlConfig.DefaultPort;
    TArray<FString> CommandLineTokens;
    TArray<FString> CommandLineSwitches;

    FCommandLine::Parse(FCommandLine::Get(), CommandLineTokens, CommandLineSwitches);

    for (FString SwitchStr : CommandLineSwitches)
    {
        FString Key;
        FString Value;

        if (SwitchStr.Split("=", &Key, &Value))
        {
            if (Key.Equals(TEXT("port"), ESearchCase::IgnoreCase))
            {
                ProcessParameters->port = FCString::Atoi(*Value);
            }
        }
    }

    //Here, the game server tells Amazon GameLift Servers where to find game session log files.
    //At the end of a game session, Amazon GameLift Servers uploads everything in the specified 
    //location and stores it in the cloud for access later.
	// 여기에서 게임 서버는 Amazon GameLift Servers에게
	// 게임 세션 로그 파일을 어디서 찾을 수 있는지 알려줍니다.
	// 게임 세션이 끝나면 Amazon GameLift Servers가 지정된 경로의 로그를 업로드하여
	// 클라우드에 저장하고, 이후 접근할 수 있게 합니다.
    TArray<FString> Logfiles;
    Logfiles.Add(TEXT("PTW/Saved/Logs/PTW.log"));
	
	FString LogFilePath = FPaths::Combine(FPaths::ProjectLogDir(), TEXT("PTW.log"));
	FString AbsoluteLogPath = FPaths::ConvertRelativePathToFull(LogFilePath);
	Logfiles.Add(AbsoluteLogPath);
    
	ProcessParameters->logParameters = Logfiles;

    //The game server calls ProcessReady() to tell Amazon GameLift Servers it's ready to host game sessions.
	// 게임 서버는 ProcessReady()를 호출하여 Amazon GameLift Servers에게 “게임 세션을 호스팅할 준비가 완료되었다”는 것을 알립니다.
    UE_LOG(GameServerLog, Log, TEXT("Calling Process Ready..."));
    FGameLiftGenericOutcome ProcessReadyOutcome = GameLiftSdkModule->ProcessReady(*ProcessParameters);

    if (ProcessReadyOutcome.IsSuccess())
    {
        UE_LOG(GameServerLog, SetColor, TEXT("%s"), COLOR_GREEN);
        UE_LOG(GameServerLog, Log, TEXT("Process Ready!"));
        UE_LOG(GameServerLog, SetColor, TEXT("%s"), COLOR_NONE);
    }
    else
    {
        UE_LOG(GameServerLog, SetColor, TEXT("%s"), COLOR_RED);
        UE_LOG(GameServerLog, Log, TEXT("ERROR: Process Ready Failed!"));
        FGameLiftError ProcessReadyError = ProcessReadyOutcome.GetError();
        UE_LOG(GameServerLog, Log, TEXT("ERROR: %s"), *ProcessReadyError.m_errorMessage);
        UE_LOG(GameServerLog, SetColor, TEXT("%s"), COLOR_NONE);
    }

    UE_LOG(GameServerLog, Log, TEXT("InitGameLift completed!"));
}
#endif
